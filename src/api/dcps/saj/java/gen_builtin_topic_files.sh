# This hard code won't necessarily do what it
# says on the tin if you change IDL contents. Beware ! @todo

# Added $OSPL_HOME/src/api/dcps/saj/java/header.txt - previously ""

cygwin=false;
case "`uname`" in
  CYGWIN*) cygwin=true;
esac
 
OUT_DIR=$OSPL_HOME/src/api/dcps/saj/java

insert_header_and_move()
{
    file=$1

    printf "Generating %s..." $file
    cat $OUT_DIR/header.txt >$OUT_DIR/tmp.txt
    cat $OUT_DIR/DDS/$file >> $OUT_DIR/tmp.txt
    if $cygwin; then
        dos2unix $OUT_DIR/tmp.txt # remove spurious ^M's
    fi
    rm -f $OUT_DIR/DDS/$file
    mv $OUT_DIR/tmp.txt $OUT_DIR/code/DDS/$file
    printf "done\n"
}

cd $OUT_DIR
idlpp -l java -S code/dds_builtinTopics.idl

cd DDS

for i in *.java; do
    insert_header_and_move $i
done

cd ..

rmdir $OUT_DIR/DDS

if test -d $OUT_DIR/DDS
then
        echo -e "\n\nSomething has gone wrong; $OUT_DIR/DDS still exists!!\n"
else
        echo -e "\n\nAll done!\n"
fi

