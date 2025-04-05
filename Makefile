all:
	g++ -std=c++17 -o main main.cpp && ./main

clean:
	rm -f main
	rm -f *.o
	rm -f *.gch
	rm -f *.dSYM
	rm -f *.out
	rm -f *.exe
	rm -f *.class
	rm -rf __pycache__
	rm -rf .pytest_cache
	rm -rf .mypy_cache
	rm -rf .coverage*
	rm -rf .pytest*
	rm -rf .tox
	rm -rf .venv
	rm -rf .env
	rm -rf .idea
	rm -rf .vscode
	rm -rf .DS_Store