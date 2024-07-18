/*
When the wave equation is discretized in the spactial domain then the equation turns into a system of ordinary differential equations which can be integrated using methods for ODEs. The system is of the form x'' = Ax.
Not sure, but I don't think the stability depends on the ODE solution method if the method is stable. I probably only depends on the eigenvalues of A. 

From (1)
We also mention the digital waveguide methods for the solution of the wave equation. These methods are finite difference schemes with a digital signal processing point of view. The signal processing approach has many favorable qualities; these include e.g. the possiblity of using frequency-dependent boundary conditions implemented by digital filters. The interpolated waveguide mesh of Savioja et al. bears some resemblance to the finite element method. See (Savioja, 1999) for an in-depth treatment of digital waveguide methods.

(1) https://www.cs.unm.edu/~williams/cs530/wave_eqn.pdf
*/

/*

*/

void runWaveEquationDemo();