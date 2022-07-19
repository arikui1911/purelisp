require 'rake/loaders/makefile'
Rake.application.add_loader 'd', Rake::MakefileLoader.new

TARGET = "src/purelisp"

SRC = FileList["src/**/*.c"]
OBJ = SRC.ext(".o")
DEP = SRC.ext(".d")

INCLUDE = ['include']

def compile_executable(dest, objs)
  sh 'gcc', '-o', dest, *objs
end

def compile_sourcefile(dest, src)
  sh 'gcc', *INCLUDE.map{|x| "-I#{x}" }, '-c', src, '-o', dest
end

def generate_dependency(dest, src)
  sh 'gcc', *INCLUDE.map{|x| "-I#{x}" }, '-MM', '-c', src, '-o', dest
end

desc "Build #{TARGET}"
task 'build' => TARGET

file TARGET => OBJ do |t|
  compile_executable t.name, t.prerequisites
end

OBJ.sort.zip(DEP.sort).each do |obj, dep|
  task obj => dep
end

desc "Generate dependency file"
task :dep => DEP

import(*DEP.select(&File.method(:exist?)))

rule '.o' => '.c' do |t|
  compile_sourcefile t.name, t.source
end

rule '.d' => '.c' do |t|
  generate_dependency t.name, t.source
end

require 'rake/clean'
CLEAN.include OBJ, DEP
CLOBBER.include TARGET


