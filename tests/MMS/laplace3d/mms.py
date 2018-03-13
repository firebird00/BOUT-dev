from __future__ import print_function
from builtins import str
from sympy import symbols, cos, sin, tan, exp, log, pi
from boutdata.mms_alternate import *
from sys import exit

global metric

# Define non-trivial metrics

geometry = SimpleTokamak()
#geometry = ShapedTokamak()

metric.__init__() # identity metric
metric.g11 = 2.+.01*x
#metric.g11 = 2.
metric.g22 = 5.
metric.g33 = 3.
#metric.g33 = 3.+y
#metric.g33 = 6.
metric.g12 = 0.7
metric.g13 = .1
metric.g23 = 0.4

# Define manufactured solution

#solution = sin(x*pi)*exp(sin(3*z)) + tan(sin(2*x*pi))*log(2. + sin(z + 1.7))
solution = sin(x*pi)

# Calculate input

input = Delp2(solution)

# Print

print("solution = "+exprToStr(solution))
print("input = "+exprToStr(input))

geometry.print_mesh()

exit(0)
