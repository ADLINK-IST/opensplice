# usage ./git_add_all_RUN.sh list language
if [ "$2" = "" ]; then 
  echo "*** usage : ./git_add_all_RUN.sh list language"
  exit;
fi
LIST=$1
LANG=$2

if [ "$LANG" = "Java" ]; then 
  CORBA_DIR=Corba/JacORB/Windows/Bat
  SA_DIR=Standalone/Windows/Bat
else
  CORBA_DIR=Corba/OpenFusion/VS2005/Bat
  SA_DIR=Standalone/VS2005/Bat
fi 
DCPS=$PWD/../../examples/dcps
for each in `cat $LIST`; do
   echo ============================================
   echo === $each;
   echo ============================================
   git add $DCPS/$each/$LANG/$CORBA_DIR/*.bat
   git add $DCPS/$each/$LANG/$SA_DIR/*.bat
done