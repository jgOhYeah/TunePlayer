# Makefile for generating the class diagram and pipeline svg images from mermaid files.
# Written by Jotham Gates
# Created 07/07/2022
# Modified 07/07/2022

.PHONY: all class-diagram pipeline

all: class-diagram pipeline

# (uint16_t* as a return type misbehaves, so substitute it in later using sed)
class-diagram:
	@echo Generating class diagram
	mmdc -i ClassDiagram.mmd -o ClassDiagram.svg
	sed -i 's/REPLACE_WITH_ASTRIX/*/g' ClassDiagram.svg

pipeline:
	@echo
	@echo Generating pipeline diagram
	mmdc -i Pipeline.mmd -o Pipeline.svg