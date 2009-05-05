import os

""" This script is written in Python 2.5 and it should works on Python 3.0 """

def Convert( src, dst, replacement ):

    print( "converting '%s' ..." % ( src ) )
    
    fin = open( src, "r" )
    fout = open( dst, "w" )
    
    for line in fin:
        for r in replacement:
            line = line.replace( r[0], r[1] )
        
        fout.write( line )
    
    fout.close()
    fin.close()

def ConvertVCProj( src ):
    Convert( src, os.path.splitext( src )[0] + ".vc8.vcproj", [ ('Version="9.00"', 'Version="8.00"') ] )
    
def ConvertSln( src ):
    Convert \
        ( src \
        , os.path.splitext( src )[0] + ".vc8.sln" \
        , [ ('Version 10.00', 'Version 9.00'), ('# Visual Studio 2008', '# Visual Studio 2005'), ('.vcproj', '.vc8.vcproj') ] \
        )

if __name__ == "__main__":

    # find all '.sln' and .'vcproj', replace their contents and save them as '.vc8.*'
    startDir = os.path.abspath( ".." )
    
    for dirname, dirs, files in os.walk( startDir, True ):
    
        if ".svn" in dirs:
            dirs.remove( ".svn" )
        
        for f in files:
            base, ext = os.path.splitext( f )
            
            if ( ext == ".vcproj" and not base.endswith( ".vc8" ) ):
                ConvertVCProj( os.path.join( dirname, f ) )
            elif ( ext == ".sln" and not base.endswith( ".vc8" ) ):
                ConvertSln( os.path.join( dirname, f ) )