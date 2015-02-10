all:
	cp files/hydrax.img hydrax.img
	./build.sh

clean:
	./clean.sh

run:
	cp files/hydrax.img hydrax.img
	./run.sh


