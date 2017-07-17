default:
	@cat makefile

clean:
	rm -f */busybusio.*
	rm -f */a.out
	rm -f */*.class
	rm -f sub_*/main_io.cpp
	rm -f $$(ls sub_01/*.java | grep -v AI_MY)
	rm -fr */__pycache__

compile:
	cd wrap_cpp; g++ -std=c++11 main_io.cpp;
	cd wrap_java; javac Main.java;
	for d in sub_03 sub_04 sub_06 sub_09; do cd $$d; cp ../wrap_cpp/main_io.cpp .; g++ -std=c++11 main_io.cpp; cd ..; done;
	for d in sub_01; do cd $$d; cp $$(ls ../wrap_java/*.java | grep -v AI_MY) .; javac Main.java; cd ..; done;

compare:
	python3 compare.py 1> compare_out.log 2> compare_err.log &

top:
	top -c -p $$(pgrep -d',' -f "[c]ompare.py")

