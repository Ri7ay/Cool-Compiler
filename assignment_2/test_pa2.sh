if [ -f results ] 
then
    rm results
fi

for file in lexer-tests/*
do
    diff <(lexer "$file") <(../build/Lexer "$file") >> results
done

if [ -s results ]
then 
    # The file is not-empty.
    echo "BAD :("
else
    # The file is empty.
    echo "GOOD!"
fi