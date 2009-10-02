#include "Pch.h"
#include "ModelImporter.h"

#include "FbxFile.h"
#include "FbxMeshAdaptors.h"
#include "FbxNodeAdaptors.h"
#include "FbxMaterialAdaptors.h"

#include "../../MCD/Core/System/ResourceManager.h"
#include "../../MCD/Render/EditableMesh.h"
#include "../../MCD/Render/MeshBuilder.h"
#include "../../MCD/Render/Material.h"
#include "../../MCD/Render/MaterialProperty.h"

#include <list>
#include <map>
#include <vector>
#include <algorithm>

// this is need for Texture property :-<
// TODO: try to hide this for cross API
#include "../../3Party/glew/glew.h"

namespace MCD
{

class FbxMeshConverter
{
private:
	// types
	struct Vertex
	{
		Vec3f pos;
		Vec3f normal;
		//Vec3f tangent;
		//Vec4f color;
		Vec2f uv0;
		Vec2f uv1;
	};

	struct Record { Vertex v; size_t idx; };

	typedef std::list<Record> RecordList;
	typedef std::map<int, RecordList> Cache;

	// member attributes
	struct IndexedMeshBuilder
	{
		Cache mCache;
		std::vector<Vertex> mVertexArray;
		std::vector<size_t> mIndexArray;
		
		// private methods
		size_t addVertex(int hash, const Vertex& vertex)
		{
			RecordList& recList = mCache[hash];

			for(RecordList::iterator ivert = recList.begin(); ivert != recList.end(); ++ivert)
			{
				if(::memcmp(&ivert->v, &vertex, sizeof(Vertex)) == 0)
					return ivert->idx;
			}

			Record rec;
			rec.v = vertex;
			rec.idx = mVertexArray.size();

			recList.push_back(rec);
			mVertexArray.push_back(vertex);
			
			return rec.idx;
		}
	};

	IndexedMeshBuilder mIdxMesh;
	FbxMeshAdaptor mFbxMesh;
	FbxNodeAdaptor mFbxNode;
	IResourceManager* mResMgr;
	Path mRootPath;

public:
	FbxMeshConverter(KFbxNode* meshNode, IResourceManager* resMgr, const Path& rootPath)
		: mFbxMesh((KFbxMesh*)meshNode->GetNodeAttribute())
		, mFbxNode(meshNode)
		, mResMgr(resMgr)
		, mRootPath(rootPath)
	{
		// read geometry
		const int cFaceCnt = mFbxMesh.getFaceCount();

		for(int iface = 0; iface < cFaceCnt; ++iface)
		{
			for(int ivert = 0; ivert < 3; ++ivert)
			{
				int hash = mFbxMesh.getKFbxMesh()->GetPolygonVertex(iface, ivert);

				Vertex vert;

				double px, py, pz, nx, ny, nz, u0, v0, u1, v1, cr, cg, cb, ca;

				mFbxMesh.getPosition(iface, ivert, px, py, pz);
				mFbxMesh.getNormal(iface, ivert, nx, ny, nz);
				mFbxMesh.getUV(iface, ivert, 0, u0, v0);
				mFbxMesh.getUV(iface, ivert, 1, u1, v1);
				mFbxMesh.getVertexColor(iface, ivert, cr, cg, cb, ca);
				
				vert.pos = Vec3f((float)px, (float)py, (float)pz);
				vert.normal = Vec3f((float)nx, (float)ny, (float)nz);
				vert.uv0 = Vec2f((float)u0, (float)v0);
				vert.uv1 = Vec2f((float)u1, (float)v1);
				//vert.color = Vec4f(cr, cg, cb, ca);

				size_t vertIdx = mIdxMesh.addVertex(hash, vert);
				mIdxMesh.mIndexArray.push_back(vertIdx);
			}
		}

		// read mesh transform
		KFbxMatrix matrix = mFbxNode.getGlobalTransform(-1) * mFbxNode.getGeometryTransform();
		Mat44f transform;

		for(int icol=0; icol<4; ++icol)
		{
			transform.r0[icol] = (float)matrix.Get(0, icol);
			transform.r1[icol] = (float)matrix.Get(1, icol);
			transform.r2[icol] = (float)matrix.Get(2, icol);
			transform.r3[icol] = (float)matrix.Get(3, icol);
		}

		transform = transform.transpose();

		for(size_t ivert = 0; ivert < mIdxMesh.mVertexArray.size(); ++ivert)
		{
			Vertex &vert = mIdxMesh.mVertexArray[ivert];
			transform.transformPoint(vert.pos);
			transform.transformNormal(vert.normal);
			//transform.transformNormal(vert.tangent);
		}
	}

	Material* importMaterial(KFbxSurfaceMaterial* fbxMtl)
	{
		Material* material = new Material;

		FbxPhongMaterialAdaptor phong(fbxMtl);

        if(phong.valid())
        {
			material->addProperty
				( new StandardProperty
					( phong.getAmbientColor(), phong.getDiffuseColor(), phong.getSpecularColor()
					, ColorProperty::ColorOperation::Replace
					, phong.getShininess() )
				, 0);

			if(nullptr != mResMgr && phong.hasDiffuseTexture())
			{
				Path texpath(strToWStr(phong.getDiffuseTextureFilename()));

				Texture* texture = (Texture*)mResMgr->load(mRootPath / texpath.getLeaf()).get();
				material->addProperty
					( new TextureProperty(texture, 0, GL_LINEAR, GL_LINEAR)
					, 0);
			}
		}
		else
		{
			material->addProperty
				( new StandardProperty
					( MCD::ColorRGBAf(1, 1, 1, 1)
					, MCD::ColorRGBAf(1, 1, 1, 1)
					, MCD::ColorRGBAf(1, 1, 1, 1)
					, ColorProperty::ColorOperation::Replace
					, 16.0f)
				, 0);
		}

		return material;
	}

	void toMeshes(Model& outModel, bool editable)
	{
		MeshBuilderPtr builder = new MeshBuilder(false);
		if(mIdxMesh.mVertexArray.size() >= 0x0000ffff)
		{
			Log::format(Log::Error, L"TriangleBuffer: too many vertices %d (max. %d)!!", mIdxMesh.mVertexArray.size(), 0xffff);
			return;
		}

		builder->enable
			( Mesh::Position | Mesh::Normal	// position, normal
			| Mesh::TextureCoord1			// uv0, uv1
			| Mesh::Index);

		builder->textureUnit(Mesh::TextureCoord0);
		builder->textureCoordSize(2);

		builder->textureUnit(Mesh::TextureCoord1);
		builder->textureCoordSize(2);

		//builder->textureUnit(Mesh::TextureCoord2);
		//builder->textureCoordSize(3);

		//builder->textureUnit(Mesh::TextureCoord3);
		//builder->textureCoordSize(4);

		for(size_t ivert = 0; ivert < mIdxMesh.mVertexArray.size(); ++ivert)
		{
			Vertex &vert = mIdxMesh.mVertexArray[ivert];

			builder->position(vert.pos);
			builder->normal(vert.normal);

			builder->textureUnit(Mesh::TextureCoord0);
			builder->textureCoord(vert.uv0);

			builder->textureUnit(Mesh::TextureCoord1);
			builder->textureCoord(vert.uv1);

			//builder->textureUnit(Mesh::TextureCoord2);
			//builder->textureCoord(vert.tangent);

			//builder->textureUnit(Mesh::TextureCoord3);
			//builder->textureCoord(vert.color);

			uint16_t idx = builder->addVertex();

			MCD_VERIFY(idx == (uint16_t)ivert);
		}

		MCD_VERIFY(0 == mIdxMesh.mIndexArray.size() % 3);

		for(size_t iface = 0; iface < mIdxMesh.mIndexArray.size();)
		{
			uint16_t i0 = uint16_t(mIdxMesh.mIndexArray[iface++]);
			uint16_t i1 = uint16_t(mIdxMesh.mIndexArray[iface++]);
			uint16_t i2 = uint16_t(mIdxMesh.mIndexArray[iface++]);
			builder->addTriangle(i0, i1, i2);
		}

		// commit to mesh
		Model::MeshAndMaterial* m = new Model::MeshAndMaterial;

		m->mesh = new EditableMesh;
		builder->commit(*m->mesh, MeshBuilder::Static);
		((EditableMesh*)(m->mesh.get()))->builder = builder;

		m->material.reset(
			importMaterial(
				mFbxNode.getKFbxNode()->GetMaterialCount() > 0
				? mFbxNode.getKFbxNode()->GetMaterial(0)
				: nullptr));
		
		outModel.mMeshes.pushBack(*m);
	}

	void toMeshesWithMaterials(Model& outModel, bool editable)
	{
		const int cMtlCnt = mFbxNode.getKFbxNode()->GetMaterialCount();

		if(cMtlCnt < 2)
		{
			toMeshes(outModel, editable);
			return;
		}

		// A - vertex in new mesh(with matching material id)
		// B - vertex in original mesh
		std::vector<size_t> BtoA(mIdxMesh.mVertexArray.size());
		std::vector<size_t> AtoB;
		AtoB.reserve(mIdxMesh.mIndexArray.size());

		for(int imtl = 0; imtl < cMtlCnt; ++imtl)
		{
			MeshBuilderPtr builder = new MeshBuilder(false);

			AtoB.clear();

			// collect all faces which used this material
			const int cFaceCnt = mFbxMesh.getFaceCount();

			for(int iface = 0; iface < cFaceCnt; ++iface)
			{
				if(mFbxMesh.getMaterialId(iface) == imtl)
				{
					size_t idx0 = mIdxMesh.mIndexArray[iface*3+0];
					size_t idx1 = mIdxMesh.mIndexArray[iface*3+1];
					size_t idx2 = mIdxMesh.mIndexArray[iface*3+2];
					AtoB.push_back(idx0);
					AtoB.push_back(idx1);
					AtoB.push_back(idx2);
				}
			}

			std::sort(AtoB.begin(), AtoB.end());
			std::vector<size_t>::iterator e = std::unique(AtoB.begin(), AtoB.end());
			const size_t cVertexCnt = std::distance(AtoB.begin(), e);

			if(cVertexCnt >= 0xffff)
			{
				Log::format(Log::Error, L"Mesh: too many vertices %d (max. %d)!!", cVertexCnt, 0xffff);
				continue;
			}

			if(cVertexCnt == 0)
			{
				continue;
			}

			for(size_t i = 0; i < cVertexCnt; ++i)
				BtoA[AtoB[i]] = i;

			// convert to MeshBuilder
			builder->enable
				( Mesh::Position | Mesh::Normal	// position, normal
				| Mesh::TextureCoord1			// uv0, uv1, tangent, and color
				| Mesh::Index);

			builder->textureUnit(Mesh::TextureCoord0);
			builder->textureCoordSize(2);

			builder->textureUnit(Mesh::TextureCoord1);
			builder->textureCoordSize(2);

			//builder->textureUnit(Mesh::TextureCoord2);
			//builder->textureCoordSize(3);

			//builder->textureUnit(Mesh::TextureCoord3);
			//builder->textureCoordSize(4);

			for(size_t ivert = 0; ivert < cVertexCnt; ++ivert)
			{
				Vertex &vert = mIdxMesh.mVertexArray[AtoB[ivert]];

				builder->position(vert.pos);
				builder->normal(vert.normal);

				builder->textureUnit(Mesh::TextureCoord0);
				builder->textureCoord(vert.uv0);

				builder->textureUnit(Mesh::TextureCoord1);
				builder->textureCoord(vert.uv1);

				//builder->textureUnit(Mesh::TextureCoord2);
				//builder->textureCoord(vert.tangent);

				//builder->textureUnit(Mesh::TextureCoord3);
				//builder->textureCoord(vert.color);

				uint16_t idx = builder->addVertex();

				MCD_VERIFY(idx == (uint16_t)ivert);
			}

			for(int iface = 0; iface < cFaceCnt; ++iface)
			{
				if(mFbxMesh.getMaterialId(iface) == imtl)
				{
					size_t i0 = mIdxMesh.mIndexArray[iface*3+0];
					size_t i1 = mIdxMesh.mIndexArray[iface*3+1];
					size_t i2 = mIdxMesh.mIndexArray[iface*3+2];
				
					builder->addTriangle(uint16_t(BtoA[i0]), uint16_t(BtoA[i1]), uint16_t(BtoA[i2]));
				}
			}

			// commit to Mesh
			Model::MeshAndMaterial* m = new Model::MeshAndMaterial;

			m->material.reset(importMaterial(mFbxNode.getKFbxNode()->GetMaterial(imtl)));
			if(editable)
			{
				m->mesh = new EditableMesh;
				((EditableMesh*)(m->mesh.get()))->builder = builder;
			}
			else
			{
				m->mesh = new Mesh;
			}

			builder->commit(*m->mesh, MeshBuilder::Static);
				
			outModel.mMeshes.pushBack(*m);
		}
	}

};	// FbxMeshConverter

ModelImporter::ModelImporter(IResourceManager* resMgr)
	: mResMgr(resMgr)
{
}

void ModelImporter::import(FbxFile& fbxfile, Model& outModel, bool editable)
{
	KArrayTemplate<KFbxNode*> meshNodes;
	fbxfile.scene()->FillNodeArray(meshNodes, KFbxNodeAttribute::eMESH);

	const int meshCnt = meshNodes.GetCount();

	Path rootPath(strToWStr(fbxfile.filePath()));
	rootPath = rootPath.getBranchPath();

	for(size_t imesh = 0; imesh < meshCnt; ++imesh)
	{
		FbxMeshConverter converter(meshNodes.GetAt(imesh), mResMgr, rootPath);
		converter.toMeshesWithMaterials(outModel, editable);
	}
}

}	// namespace MCD
