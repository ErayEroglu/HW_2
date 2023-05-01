; ModuleID = 'advcalc2ir'
declare i32 @printf(i8*, ...)
@print.str = constant [4 x i8] c"%d\0A\00"

define i32 @main() {
%x = alloca i32
store i32 1, i32* %x
%y = alloca i32
%1 = load i32, i32* %x
%2 = add i32 %1,3
store i32 %2, i32* %y
%z = alloca i32
%3 = load i32, i32* %x
%4 = load i32, i32* %y
%5 = load i32, i32* %y
%6 = load i32, i32* %y
%7 = mul i32 %3,%4
%8 = mul i32 %7,%5
%9 = mul i32 %8,%6
store i32 %9, i32* %z
%10 = load i32, i32* %z
call i32 (i8*, ...) @printf(i8* getelementptr ([4 x i8], [4 x i8]* @print.str, i32 0, i32 0), i32 %10)
%12 = load i32, i32* %x
%13 = load i32, i32* %x
%14 = xor i32 %12,%13
call i32 (i8*, ...) @printf(i8* getelementptr ([4 x i8], [4 x i8]* @print.str, i32 0, i32 0), i32 %14)
%16 = load i32, i32* %x
%17 = load i32, i32* %x
%18 = load i32, i32* %z
%19 = load i32, i32* %y
%20 = xor i32 %16,%17
%21 = add i32 %18,%19
%22 = or i32 %20,%21
call i32 (i8*, ...) @printf(i8* getelementptr ([4 x i8], [4 x i8]* @print.str, i32 0, i32 0), i32 %22)
%24 = load i32, i32* %x
%25 = load i32, i32* %x
%26 = load i32, i32* %z
%27 = load i32, i32* %y
%28 = xor i32 %24,%25
%29 = add i32 %26,%27
%30 = or i32 %28,%29
%31 = ashr i32 %30,1
call i32 (i8*, ...) @printf(i8* getelementptr ([4 x i8], [4 x i8]* @print.str, i32 0, i32 0), i32 %31)
%33 = load i32, i32* %x
%34 = load i32, i32* %x
%35 = load i32, i32* %z
%36 = load i32, i32* %y
%37 = xor i32 %33,%34
%38 = add i32 %35,%36
%39 = or i32 %37,%38
%40 = ashr i32 %39,1
%41 = shl i32 %40,1
call i32 (i8*, ...) @printf(i8* getelementptr ([4 x i8], [4 x i8]* @print.str, i32 0, i32 0), i32 %41)
ret i32 0
}