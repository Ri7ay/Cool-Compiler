if [ -f results ] 
then
    rm results
fi

for file in /home/zakhar/MyGits/Cool-Compiler/assignment_2/lexer-tests/*
do
    diff <(lexer "$file") <(./Lexer "$file") >> results
done

if [ -s results ]
then 
    # The file is not-empty.
    echo "BAD :("
else
    # The file is empty.
    echo "GOOD!"
fi