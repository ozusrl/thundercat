import os
import glob
import sys

testCases = [
    ("fidap005b", "splitAll unrolling 5 CSRbyNZ"),
    ("fidap005b", "splitAll unrolling 3 CSRbyNZ"),
    ("fidap005b", "splitAll unrolling 1 CSRbyNZ"),
    ("fidap005b", "splitAll CSRbyNZ CSRbyNZ"),
    ("fidap005b", "splitAll OSKI 3 3 CSRbyNZ"),
    ("fidap005b", "splitAll OSKI 3 9 CSRbyNZ"),
    ("fidap005b", "splitAll OSKI 9 3 CSRbyNZ"),
    ("fidap005b", "splitAll stencil CSRbyNZ"),
    ("fidap005b", "splitAll unfolding 100000 100000 CSRbyNZ"),
    ("fidap005b", "splitAll unfolding 3 3 CSRbyNZ"),
    ("fidap005b", "splitAll unfolding 4 5 CSRbyNZ"),
    ("fidap005b", "splitAll unfolding 1 1 CSRbyNZ"),
    ("fidap005b", "splitByBand 26 26 diagonal CSRbyNZ"),
    ("fidap005b", "splitByDenseBand 26 26 0 diagonal CSRbyNZ"),

    ("fidap005b", "splitByCount 100 unrolling 3 unrolling 3"),
    ("fidap005b", "splitByCount 100 unrolling 1 CSRbyNZ"),
    ("fidap005b", "splitByCount 100 unrolling 4 OSKI 3 3"),
    ("fidap005b", "splitByCount 50 unrolling 5 stencil"),
    ("fidap005b", "splitByCount 105 unrolling 5 unfolding 10000 10000"),
    ("fidap005b", "splitByCount 106 CSRbyNZ unrolling 4"),
    ("fidap005b", "splitByCount 200 CSRbyNZ CSRbyNZ"),
    ("fidap005b", "splitByCount 105 CSRbyNZ OSKI 9 3"),
    ("fidap005b", "splitByCount 205 CSRbyNZ stencil"),
    ("fidap005b", "splitByCount 105 CSRbyNZ unfolding 9 9"),
    ("fidap005b", "splitByCount 15  OSKI 3 3 unrolling 5"),
    ("fidap005b", "splitByCount 205 OSKI 3 3 CSRbyNZ"),
    ("fidap005b", "splitByCount 105 OSKI 3 3 OSKI 9 9"),
    ("fidap005b", "splitByCount 105 OSKI 9 3 stencil"),
    ("fidap005b", "splitByCount 105 OSKI 3 9 unfolding 9 7"),
    ("fidap005b", "splitByCount 15  stencil unrolling 5"),
    ("fidap005b", "splitByCount 205 stencil CSRbyNZ"),
    ("fidap005b", "splitByCount 105 stencil OSKI 9 9"),
    ("fidap005b", "splitByCount 150 stencil stencil"),
    ("fidap005b", "splitByCount 105 stencil unfolding 11 7"),
    ("fidap005b", "splitByCount 105 unfolding 10000 10000 unrolling 4"),
    ("fidap005b", "splitByCount 105 unfolding 7 10 CSRbyNZ"),
    ("fidap005b", "splitByCount 105 unfolding 9 10 OSKI 3 3"),
    ("fidap005b", "splitByCount 111 unfolding 3 3 stencil"),
    ("fidap005b", "splitByCount 105 unfolding 9 9 unfolding 10000 10000"),
    ("fidap005b", "splitByBand 0 0 diagonal CSRbyNZ"),
    ("fidap005b", "splitByBand 1 1 diagonal CSRbyNZ"),
    ("fidap005b", "splitByBand 3 5 diagonal CSRbyNZ"),
    ("fidap005b", "splitByBand 5 5 diagonal CSRbyNZ"),
    ("fidap005b", "splitByDenseBand 0 0 100 diagonal CSRbyNZ"),
    ("fidap005b", "splitByDenseBand 1 1 90 diagonal CSRbyNZ"),
    ("fidap005b", "splitByDenseBand 3 5 80 diagonal CSRbyNZ"),
    ("fidap005b", "splitByDenseBand 3 15 50 diagonal CSRbyNZ"),
    ("fidap005b", "splitByDenseBand 5 5 50 diagonal CSRbyNZ"),
    ("fidap005b", "splitByDenseBand 5 5 0 diagonal CSRbyNZ"),

    ("add32b", "splitAll unrolling 5 CSRbyNZ"),
    ("add32b", "splitAll CSRbyNZ CSRbyNZ"),
    ("add32b", "splitAll OSKI 4 2 CSRbyNZ"),
    ("add32b", "splitAll OSKI 2 4 CSRbyNZ"),
    ("add32b", "splitAll stencil CSRbyNZ"),
    ("add32b", "splitAll unfolding 100000 100000 CSRbyNZ"),
    ("add32b", "splitAll unfolding 4 4 CSRbyNZ"),
    ("add32b", "splitAll unfolding 3 7 CSRbyNZ"),
    ("add32b", "splitByBand 0 0 diagonal CSRbyNZ"),
    ("add32b", "splitByBand 1 1 diagonal CSRbyNZ"),
    ("add32b", "splitByBand 3 5 diagonal CSRbyNZ"),
    ("add32b", "splitByBand 5 5 diagonal CSRbyNZ"),

    ("memplusb", "splitAll unrolling 5 CSRbyNZ"),
    ("memplusb", "splitAll CSRbyNZ CSRbyNZ"),
    ("memplusb", "splitAll OSKI 13 2 CSRbyNZ"),
    ("memplusb", "splitAll OSKI 2 2 CSRbyNZ"),
    ("memplusb", "splitAll OSKI 2 13 CSRbyNZ"),
    ("memplusb", "splitAll stencil CSRbyNZ"),
    ("memplusb", "splitAll unfolding 100000 100000 CSRbyNZ"),
    ("memplusb", "splitAll unfolding 4 2 CSRbyNZ"),
    ("memplusb", "splitAll unfolding 1 1 CSRbyNZ"),
    ("memplusb", "splitByBand 0 0 diagonal unrolling 1"),
    ("memplusb", "splitByBand 1 1 diagonal unrolling 1"),
    ("memplusb", "splitByBand 3 5 diagonal unrolling 1"),
]

def runTestCase(matrixName, arguments):
    print "Testing ", matrixName, "  ", arguments
    outputFile = matrixName + "_" + arguments.replace(' ', '_') + ".out"
    testCmd = ("../spMVgen " + "matrices/" + matrixName + " " + arguments 
               + " 2> /dev/null > " + outputFile)
    if(os.system(testCmd) != 0):
        print testCmd
        print "  !!! Problem executing the test."
        return
    else:
        os.system("diff correctOutputs/"+matrixName+".out "+outputFile)

def runAllTests():
    for case in testCases:
      runTestCase(case[0], case[1])
    return

def main():
    os.system("rm -f *.out")
    if (len(sys.argv) > 1):
        runTestCase(sys.argv[1], ' '.join(sys.argv[2:]))
    else:
        runAllTests()
    return

if __name__ == "__main__":
    main()
