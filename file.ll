; ModuleID = 'advcalc2ir'
declare i32 @printf(i8*, ...)
@print.str = constant [4 x i8] c"%d\0A\00"

define i32 @main() {
%a = alloca i32
store i32 15, i32* %a
%b = alloca i32
%1 load i32, i32* %a
%2 load i32, i32* %b
%3 = add i32 %1,%2
call i32 (i8*, ...) @printf(i8* getelementptr ([4 x i8], [4 x i8]* @print.str, i32 0, i32 0), i32 %3)
%c = alloca i32
%z = alloca i32
%5 load i32, i32* %c
%6 load i32, i32* %z
%7 = add i32 %5,%6
call i32 (i8*, ...) @printf(i8* getelementptr ([4 x i8], [4 x i8]* @print.str, i32 0, i32 0), i32 %7)
store i32 878, i32* %a
ret i32 0
}