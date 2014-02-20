
SHELL := /bin/bash
PWD := $(shell pwd)

all: build

.PHONY: build
build:
	python scripts/build.py all

.PHONY: clean
clean:
	echo "clean all"
	python scripts/clean.py all

.PHONY: hypervisor
hypervisor:
	echo "build hypervisor"
	python scripts/build.py hypervisor

.PHONY: hypervisorclean
hypervisorclean:
	echo "clean hypervisor"
	python scripts/clean.py hypervisor

.PHONY: guest
guest:
	echo "build guest"
	python scripts/build.py guest

.PHONY: guestclean
guestclean:
	echo "clean guest"
	python scripts/clean.py guest
