import sys
import csv

def merge(fileNames):
    if len(fileNames) == 0:
        print "You must provide fileNames."
        sys.exit()

    files = []

    for fileName in fileNames:
        files += [readFile(fileName)]

    matrixNames = getMatrixNames(files[0])
    values = []

    for file in files:
        values += [getNumbers(file)]
    
    values = zip(matrixNames, *values)

    return values

def readFile(fileName):
    file = open(fileName, 'r')
    fileContents = []

    for line in file:
        fields = line.split(',')
        fileContents += [(fields[0], int(fields[1]))]
    return fileContents

def getNumbers(file):
    return map(lambda (m,v): v, file)

def getMatrixNames(file):
    return map(lambda (m,v): m, file)

def printValues(values):
    for record in values:
        for item in record:
            print item,
        print ""

def getMinValues(files):
    minValues = []

    for record in files:
        minValues += [(record[0], min(record[1:]))]
  
    print minValues
    return minValues


def writeCSVFile(values):
    file = open("test.csv", "wb")
    fileWriter = csv.writer(file , delimiter=',',quotechar='|', quoting=csv.QUOTE_MINIMAL)

    for record in values:
        fileWriter.writerow(record)   

def prepareDataForPlot(fileNames):
    fileContent = []

    fileContent = merge(fileNames)
    print fileContent
  
    writeCSVFile(fileContent)

#printValues(merge(sys.argv[1:]))

#read some files, get minValues, dump to test.csv
#writeCSVFile(getMinValues(merge(sys.argv[1:])))

#read all files for plotting (5 from clang, 3 from spMVlib, dump to test.csv)
prepareDataForPlot(sys.argv[1:])



#commands for prepareDataForPlot
#python ~/svnBuse/llvm/spMVlib/tools/mergeCSVfiles.py turing_clang_all/turing.unrolling_2.csv turing_clang_all/oski_best.csv  turing_clang_all/turing.CSRbyNZ.csv turing_clang_all/unrolling_best.csv turing_clang_all/turing.unfolding.csv turing_clang_all/turing.stencil.csv unfolding_best.csv turing.spMVgen.CSRbyNZ.csv turing.spMVgen.stencil.csv 

#python ~/svnBuse/llvm/spMVlib/tools/mergeCSVfiles.py chicago.local_clang_all/chicago.local.unrolling_2.csv chicago.local_clang_all/oski_best.csv  chicago.local_clang_all/chicago.local.CSRbyNZ.csv chicago.local_clang_all/unrolling_best.csv chicago.local_clang_all/chicago.local.unfolding.csv chicago.local_clang_all/chicago.local.stencil.csv chicago.local_spMVlib_all/unfolding_best.csv chicago.local_spMVlib_all/chicago.local.spMVgen.CSRbyNZ.csv chicago.local_spMVlib_all/chicago.local.spMVgen.stencil.csv 
