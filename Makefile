all:
	g++ *.cpp -o concuroram -std=c++17 -pthread
	./concuroram

clean:
	rm -f main
	rm -f *.o
	rm -f *.out
	rm -f *.exe
	rm -f *.class
	rm -rf __pycache__
	rm -rf .venv
	rm -rf .env
	rm -rf .idea
	rm -rf .vscode
	rm concuroram