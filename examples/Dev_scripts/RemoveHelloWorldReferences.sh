

TargetExample=ContentFilteredTopic
for i in `find -P ../dcps/${TargetExample} -name "*HelloWorld*" -type d`; do j=`echo $i | sed "s%HelloWorld%${TargetExample}%g"`; mv $i $j; done
for i in `find -P ../dcps/${TargetExample} -name "*HelloWorld*" -type f`; do j=`echo $i | sed "s%HelloWorld%${TargetExample}%g"`; mv $i $j; done

for i in `find -P ../dcps/${TargetExample} -name "*" -type f`; do sed "s%HelloWorld%${TargetExample}%g" -i $i; done

TargetExample_LowerCase=contentFilteredTopic
for i in `find -P ../dcps/${TargetExample} -name "*" -type f`; do sed "s%helloWorld%${TargetExample_LowerCase}%g" -i $i; done
TargetExample_UpperCase=CONTENTFILTEREDTOPIC
for i in `find -P ../dcps/${TargetExample} -name "*" -type f`; do sed "s%HELLOWORLD%${TargetExample_UpperCase}%g" -i $i; done

TargetExample=Durability
for i in `find -P ../dcps/${TargetExample} -name "*HelloWorld*" -type d`; do j=`echo $i | sed "s%HelloWorld%${TargetExample}%g"`; mv $i $j; done
for i in `find -P ../dcps/${TargetExample} -name "*HelloWorld*" -type f`; do j=`echo $i | sed "s%HelloWorld%${TargetExample}%g"`; mv $i $j; done

for i in `find -P ../dcps/${TargetExample} -name "*" -type f`; do sed "s%HelloWorld%${TargetExample}%g" -i $i; done

TargetExample_LowerCase=durability
for i in `find -P ../dcps/${TargetExample} -name "*" -type f`; do sed "s%helloWorld%${TargetExample_LowerCase}%g" -i $i; done
TargetExample_UpperCase=DURABILITY
for i in `find -P ../dcps/${TargetExample} -name "*" -type f`; do sed "s%HELLOWORLD%${TargetExample_UpperCase}%g" -i $i; done

TargetExample=Lifecycle
for i in `find -P ../dcps/${TargetExample} -name "*HelloWorld*" -type d`; do j=`echo $i | sed "s%HelloWorld%${TargetExample}%g"`; mv $i $j; done
for i in `find -P ../dcps/${TargetExample} -name "*HelloWorld*" -type f`; do j=`echo $i | sed "s%HelloWorld%${TargetExample}%g"`; mv $i $j; done

for i in `find -P ../dcps/${TargetExample} -name "*" -type f`; do sed "s%HelloWorld%${TargetExample}%g" -i $i; done

TargetExample_LowerCase=lifecycle
for i in `find -P ../dcps/${TargetExample} -name "*" -type f`; do sed "s%helloWorld%${TargetExample_LowerCase}%g" -i $i; done
TargetExample_UpperCase=LIFECYCLE
for i in `find -P ../dcps/${TargetExample} -name "*" -type f`; do sed "s%HELLOWORLD%${TargetExample_UpperCase}%g" -i $i; done

TargetExample=Listener
for i in `find -P ../dcps/${TargetExample} -name "*HelloWorld*" -type d`; do j=`echo $i | sed "s%HelloWorld%${TargetExample}%g"`; mv $i $j; done
for i in `find -P ../dcps/${TargetExample} -name "*HelloWorld*" -type f`; do j=`echo $i | sed "s%HelloWorld%${TargetExample}%g"`; mv $i $j; done

for i in `find -P ../dcps/${TargetExample} -name "*" -type f`; do sed "s%HelloWorld%${TargetExample}%g" -i $i; done

TargetExample_LowerCase=listener
for i in `find -P ../dcps/${TargetExample} -name "*" -type f`; do sed "s%helloWorld%${TargetExample_LowerCase}%g" -i $i; done
TargetExample_UpperCase=LISTENER
for i in `find -P ../dcps/${TargetExample} -name "*" -type f`; do sed "s%HELLOWORLD%${TargetExample_UpperCase}%g" -i $i; done

TargetExample=Ownership
for i in `find -P ../dcps/${TargetExample} -name "*HelloWorld*" -type d`; do j=`echo $i | sed "s%HelloWorld%${TargetExample}%g"`; mv $i $j; done
for i in `find -P ../dcps/${TargetExample} -name "*HelloWorld*" -type f`; do j=`echo $i | sed "s%HelloWorld%${TargetExample}%g"`; mv $i $j; done

for i in `find -P ../dcps/${TargetExample} -name "*" -type f`; do sed "s%HelloWorld%${TargetExample}%g" -i $i; done

TargetExample_LowerCase=ownership
for i in `find -P ../dcps/${TargetExample} -name "*" -type f`; do sed "s%helloWorld%${TargetExample_LowerCase}%g" -i $i; done
TargetExample_UpperCase=OWNERSHIP
for i in `find -P ../dcps/${TargetExample} -name "*" -type f`; do sed "s%HELLOWORLD%${TargetExample_UpperCase}%g" -i $i; done

TargetExample=QueryCondition
for i in `find -P ../dcps/${TargetExample} -name "*HelloWorld*" -type d`; do j=`echo $i | sed "s%HelloWorld%${TargetExample}%g"`; mv $i $j; done
for i in `find -P ../dcps/${TargetExample} -name "*HelloWorld*" -type f`; do j=`echo $i | sed "s%HelloWorld%${TargetExample}%g"`; mv $i $j; done

for i in `find -P ../dcps/${TargetExample} -name "*" -type f`; do sed "s%HelloWorld%${TargetExample}%g" -i $i; done

TargetExample_LowerCase=queryCondition
for i in `find -P ../dcps/${TargetExample} -name "*" -type f`; do sed "s%helloWorld%${TargetExample_LowerCase}%g" -i $i; done
TargetExample_UpperCase=QUERYCONDITION
for i in `find -P ../dcps/${TargetExample} -name "*" -type f`; do sed "s%HELLOWORLD%${TargetExample_UpperCase}%g" -i $i; done

TargetExample=WaitSet
for i in `find -P ../dcps/${TargetExample} -name "*HelloWorld*" -type d`; do j=`echo $i | sed "s%HelloWorld%${TargetExample}%g"`; mv $i $j; done
for i in `find -P ../dcps/${TargetExample} -name "*HelloWorld*" -type f`; do j=`echo $i | sed "s%HelloWorld%${TargetExample}%g"`; mv $i $j; done

for i in `find -P ../dcps/${TargetExample} -name "*" -type f`; do sed "s%HelloWorld%${TargetExample}%g" -i $i; done

TargetExample_LowerCase=waitSet
for i in `find -P ../dcps/${TargetExample} -name "*" -type f`; do sed "s%helloWorld%${TargetExample_LowerCase}%g" -i $i; done
TargetExample_UpperCase=WAITSET
for i in `find -P ../dcps/${TargetExample} -name "*" -type f`; do sed "s%HELLOWORLD%${TargetExample_UpperCase}%g" -i $i; done

TargetExample=BuiltInTopics
for i in `find -P ../dcps/${TargetExample} -name "*HelloWorld*" -type d`; do j=`echo $i | sed "s%HelloWorld%${TargetExample}%g"`; mv $i $j; done
for i in `find -P ../dcps/${TargetExample} -name "*HelloWorld*" -type f`; do j=`echo $i | sed "s%HelloWorld%${TargetExample}%g"`; mv $i $j; done

for i in `find -P ../dcps/${TargetExample} -name "*" -type f`; do sed "s%HelloWorld%${TargetExample}%g" -i $i; done

TargetExample_LowerCase=builtInTopics
for i in `find -P ../dcps/${TargetExample} -name "*" -type f`; do sed "s%helloWorld%${TargetExample_LowerCase}%g" -i $i; done
TargetExample_UpperCase=BUILTINTOPICS
for i in `find -P ../dcps/${TargetExample} -name "*" -type f`; do sed "s%HELLOWORLD%${TargetExample_UpperCase}%g" -i $i; done


