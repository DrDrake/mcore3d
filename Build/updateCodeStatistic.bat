@echo Counting code...
@"./codeStatistic.py" ../MCD .h;.cpp;.inl;.inc; > codelines.txt
@"./codeStatistic.py" ../Test .h;.cpp;.inl;.inc; >> codelines.txt
@echo Code counting finished.
@echo off
pause