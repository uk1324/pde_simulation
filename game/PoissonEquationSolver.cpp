#include "PoissonEquationSolver.hpp"
#include "MatrixUtils.hpp"
/*

Poisson's equation is
Dxx u + Dyy u = f(x, y)

If f(x, y) is continous and the u is defined on a rectangular boundary by a continous function g(x, y) then the equation has a unique solution.

Rewriting Poisson's equation to finite difference
(u(i + 1, j) - 2u(i, j) + u(i - 1, j)) / h^2 + (u(i, j + 1) - 2u(i, j) + u(i, j - 1)) / k^2 = f(i, j)

Simplifying
(k^2 u(i + 1, j) - k^2 2u(i, j) + k^2 u(i - 1, j) + h^2 u(i, j + 1) - h^2 2u(i, j) + h^2 u(i, j - 1)) / h^2 k^2 = f(i, j)
(k^2 u(i + 1, j) - k^2 2u(i, j) + k^2 u(i - 1, j) + h^2 u(i, j + 1) - h^2 2u(i, j) + h^2 u(i, j - 1)) / k^2 = h^2 f(i, j)
u(i + 1, j) - 2u(i, j) + u(i - 1, j) + (h^2 / k^2) u(i, j + 1) - (h^2/k^2) 2u(i, j) + (h^2/k^2) u(i, j - 1) = h^2 f(i, j)
-2(1 + (h^2/k^2)) u(i, j) + u(i + 1, j) + u(i - 1, j) + (h^2/k^2) (u(i, j + 1) + u(i, j - 1)) = h^2 f(i, j)
let L = h^2/k^2, M = 2(1 + L), N = -h^2
-M u(i, j) + u(i + 1, j) + u(i - 1, j) + L (u(i, j + 1) + u(i, j - 1)) = -N f(i, j)
// Not sure if this (negating both sides) is needed for something but it might make the matrix have some property (like positive definiteness idk or being stric diagonally dominant).
M u(i, j) - u(i + 1, j) - u(i - 1, j) - L u(i, j + 1) - L u(i, j - 1) = N f(i, j)

The goal is to find u(i, j) for i in { 1, ... n - 1 }, j in { 1, ..., m - 1 }
The values for the i = 0, j = 0, i = n, j = m are specified in the boundary conditions
This creates a linear system of (n - 1) * (m - 1) equations.

Corner cases
if i = 1, j = 1
M u(1, 1) - u(2, 1) - u(0, 1) - L u(1, 2) - L u(1, 0) = N f(1, 1)
M u(1, 1) - u(2, 1) - L u(1, 2) = N f(1, 1) + u(0, 1) + L u(1, 0)

if i = 1, j = m - 1
M u(1, m - 1) - u(2, m - 1) - u(0, m - 1) - L u(1, m) - L u(1, m - 2) = N f(1, m - 1)
M u(1, m - 1) - u(2, m - 1) - L u(1, m - 2) = N f(1, m - 1) + u(0, m - 1) + L u(1, m)

if i = n - 1, j = 1
M u(n - 1, 1) - u(n, 1) - u(n - 2, 1) - L u(n - 1, 2) - L u(n - 1, 0) = N f(n - 1, 1)
M u(n - 1, 1) - u(n - 2, 1) - L u(n - 1, 2) = N f(n - 1, 1) + u(n, 1) + L u(n - 1, 0)

if i = n - 1, j = m - 1
M u(n - 1, m - 1) - u(n, m - 1) - u(n - 2, m - 1) - L u(n - 1, m) - L u(n - 1, m - 2) = N f(n - 1, m - 1)
M u(n - 1, m - 1) - u(n - 2, m - 1) - L u(n - 1, m - 2) = N f(n - 1, m - 1) + u(n, m - 1) + L u(n - 1, m)

Edge cases
if i = 1, j in { 2, ... m - 2 }
M u(1, j) - u(2, j) - u(0, j) - L u(1, j + 1) - L u(1, j - 1) = N f(1, j)
M u(1, j) - u(2, j) - L u(1, j + 1) - L u(1, j - 1) = N f(1, j) + g(0, j)

if i = n - 1, j in { 2, ... m - 2 }

M u(n - 1, j) - u(n, j) - u(n - 2, j) - L u(n - 1, j + 1) - L u(n - 1, j - 1) = N f(n - 1, j)
M u(n - 1, j) - u(n - 2, j) - L u(n - 1, j + 1) - L u(n - 1, j - 1) = N f(n - 1, j) + u(n, j)

if i in { 2, ... n - 2 }, j = 1
M u(i, 1) - u(i + 1, 1) - u(i - 1, 1) - L u(i, 2) - L u(i, 0) = N f(i, 1)
M u(i, 1) - u(i + 1, 1) - u(i - 1, 1) - L u(i, 2) = N f(i, 1) + L u(i, 0) 

if i in { 2, ... n - 2 }, j = m - 1
M u(i, m - 1) - u(i + 1, m - 1) - u(i - 1, m - 1) - L u(i, m) - L u(i, m - 2) = N f(i, m - 1)
M u(i, m - 1) - u(i + 1, m - 1) - u(i - 1, m - 1) - L u(i, m - 2) = N f(i, m - 1) + L u(i, m)
*/

enum class SolveGaussianEliminationOutput {
	SUCCESS,
	NO_SOLUTION
};
using Float = f32;

// solves mx = b for x
// the matrix m has to be square
// outputs the result into x
SolveGaussianEliminationOutput solveGaussianElimination(MatrixView<Float> m, MatrixView<Float> b, MatrixView<Float> x) {
	ASSERT(m.sizeX() == m.sizeY());
	const auto n = m.sizeX();
	ASSERT(b.sizeX() == 1);
	ASSERT(b.sizeY() == n);
	ASSERT(b.size() == x.size());

	// Loop to n - 1, because (n, n) already lies on the diagonal so the nth loop would do nothing.
	for (i64 i = 0; i < n - 1; i++) {
		const auto column = i;
		// max of values on and below the diagonal in column
		Float maxValue = abs(m(column, i));
		i64 maxValueRow = i;
		for (i64 row = maxValueRow + 1; row < n; row++) {
			const auto value = abs(m(column, row));
			if (value > maxValue) {
				maxValueRow = row;
				maxValue = value;
			}
		}
		// Use the max value to prevent precision loss from division by m(column, pivotRow) later.

		if (maxValue == Float(0.0)) {
			return SolveGaussianEliminationOutput::NO_SOLUTION;
		}
		swapRows(m, i, maxValueRow);
		swapRows(b, i, maxValueRow);
		const auto pivotRow = i; 

		for (i64 row = i + 1; row < n; row++) {
			// We wan't to zero m(column, row) by adding the row m(..., pivotRow) to m(..., row)
			// m(column, row) + k m(column, pivotRow) = 0
			// k m(column, pivotRow) = -m(column, row)
			// k = -m(column, row) / m(column, pivotRow)
			const auto k = -m(column, row) / m(column, pivotRow);
			addRows(m, row, pivotRow, k);
			addRows(b, row, pivotRow, k);
			m(column, row) = Float(0.0);
		}
	}

	if (m(n - 1, n - 1) == Float(0.0)) {
		return SolveGaussianEliminationOutput::NO_SOLUTION;
	}

	ASSERT(n > 0);


	// Matrix converted to upper triangular form.
	// Now back substitute.
	for (i64 i = n - 1; i >= 0; i--) {
		x(0, i) = b(0, i);
		for (i64 j = i + 1; j < n; j++) {
			x(0, i) -= m(j, i) * x(0, j);
		}
		x(0, i) /= m(i, i);
	}

	return SolveGaussianEliminationOutput::SUCCESS;
}

void solveGaussSeidel(MatrixView<Float> m, MatrixView<Float> b, MatrixView<Float> x) {
	const auto n = m.sizeY();
	ASSERT(b.sizeX() == 1);
	ASSERT(b.sizeY() == n);
	ASSERT(b.size() == x.size());

	for (i64 i = 0; i < n; i++) {
		x(0, i) = Float(0);
	}
	i32 iterationCount = 20;
	for (i32 i = 0; i < iterationCount; i++) {
		for (i64 equationI = 0; equationI < n; equationI++) {
			Float sum = b(0, equationI);
			for (i64 variableI = 0; variableI < n; variableI++) {
				if (equationI == variableI) {
					continue;
				}
				sum -= m(variableI, equationI) * x(0, variableI);
			}
			sum /= m(equationI, equationI);
			x(0, equationI) = sum;
		}
	}
}

SolveGaussianEliminationOutput solveGaussianElimination(Matrix<Float>& m, Matrix<Float>& b, Matrix<Float>& x) {
	return solveGaussianElimination(matrixViewFromMatrix(m), matrixViewFromMatrix(b), matrixViewFromMatrix(x));
}

std::optional<Matrix<Float>> solveGaussianElimination(Matrix<Float>& m, Matrix<Float>& b) {
	auto x = Matrix<Float>::uninitialized(b.size());
	const auto result = solveGaussianElimination(m, b, x);
	if (result == SolveGaussianEliminationOutput::SUCCESS) {
		return x;
	}
	return std::nullopt;
}

void gaussianEliminationTest() {
	srand(136);
	auto m = Matrix<Float>::uninitialized(5, 5);
	for (i64 j = 0; j < m.sizeY(); j++) {
		for (i64 i = 0; i < m.sizeX(); i++) {
			m(i, j) = Float(rand()) / Float(RAND_MAX);
		}
	}
	auto b = Matrix<Float>::uninitialized(1, 5);
	for (i64 i = 0; i < m.sizeY(); i++) {
		const auto a = Float(rand());
		const auto c = a / Float(RAND_MAX);
		b(0, i) = c;
	}

	matrixPrint(m);
	matrixPrint(b);

	auto x = Matrix<Float>::uninitialized(1, 5);
	{
		auto mTmp = m.clone();
		auto bTmp = b.clone();
		const auto result = solveGaussianElimination(mTmp, bTmp, x);
		ASSERT(result == SolveGaussianEliminationOutput::SUCCESS);
	}
	matrixPrint(x);

	const auto b1 = m * x;
	matrixPrint(b1);
	matrixPrint(b);

	const auto difference = b1 - b;
	matrixPrint(difference);
}

// if a matrix is positive definite then it's determinant is positive, because the determinant is the product of the eigenvalues.

#include <Put.hpp>
// Solves Dxx u + Dyy u = f(x, y) for u
// u has to have values on boundary values specified, the function finds the order values
// The corner values are not used.
void solvePoissonEquation(const MatrixView<const Float>& f, MatrixView<Float> u, const Aabb& region) {
	ASSERT(f.size() == u.size());
	const auto n = f.sizeX() - 1;
	const auto m = f.sizeY() - 1;

	const auto regionSize = region.size();
	const auto h = regionSize.x / n;
	const auto k = regionSize.y / m;

	const auto L = (h * h) / (k * k);
	const auto M = Float(2.0) * (Float(1.0) + L);
	const auto N = -(h * h);

	/*
	Example for a grid with n = 5, m = 4
	(1, 1) -> 0
	(2, 1) -> 1
	(3, 1) -> 2
	(1, 2) -> 3
	(2, 2) -> 4
	(3, 2) -> 5
	It's skips i = 0, j = 0, i = 4, n = 3, because these are specified by the boundary conditions so there are no equations (rows) for them.
	*/
	auto gridIndexToVariableIndex = [&n](i32 i, i32 j) {
		return i + (j - 1) * (n - 1) - 1;
	};

	const auto matrixSize = (n - 1) * (m - 1);
	auto matrix = Matrix<Float>::uninitialized(matrixSize, matrixSize);
	for (i32 i = 0; i < matrix.sizeX(); i++) {
		for (i32 j = 0; j < matrix.sizeY(); j++) {
			matrix(i, j) = Float(0);
		}
	}
	auto augmentedColumn = Matrix<Float>::uninitialized(1, matrixSize);

	i32 row = 0;
	auto set = [&](i32 i, i32 j, Float value) {
		matrix(gridIndexToVariableIndex(i, j), row) = value;
	};

	auto setAugmentedColumn = [&](Float value) {
		augmentedColumn(0, row) = value;
	};

	// The values are specified row by row.
	// The equations are specified in the same order as gridIndexToVariableIndex increases.
	{
		// M u(1, 1) - u(2, 1) - L u(1, 2) = N f(1, 1) + u(0, 1) + L u(1, 0)
		set(1, 1, M);
		set(2, 1, Float(-1.0));
		set(1, 2, -L);
		setAugmentedColumn(N * f(1, 1) + u(0, 1) + L * u(1, 0));
		row++;
	}
	for (i32 i = 2; i <= n - 2; i++) {
		// M u(i, 1) - u(i + 1, 1) - u(i - 1, 1) - L u(i, 2) = N f(i, 1) + L u(i, 0)
		set(i, 1, M);
		set(i + 1, 1, Float(-1.0));
		set(i - 1, 1, Float(-1.0));
		set(i, 2, -L);
		setAugmentedColumn(N * f(i, 1) + L * u(i, 0));
		row++;
	}
	{
		// M u(n - 1, 1) - u(n - 2, 1) - L u(n - 1, 2) = N f(n - 1, 1) + u(n, 1) + L u(n - 1, 0)
		set(n - 1, 1, M);
		set(n - 2, 1, Float(-1.0));
		set(n - 1, 2, -L);
		setAugmentedColumn(N * f(n - 1, 1) + u(n, 1) + L * u(n - 1, 0));
		row++;
	}
	for (i32 j = 2; j <= m - 2; j++) {
		{
			// M u(1, j) - u(2, j) - L u(1, j + 1) - L u(1, j - 1) = N f(1, j) + u(0, j)
			set(1, j, M);
			set(2, j, Float(-1.0));
			set(1, j + 1, -L);
			set(1, j - 1, -L);
			setAugmentedColumn(N * f(1, j) + u(0, j));
			row++;
		}
		for (i32 i = 2; i <= n - 2; i++) {
			// M u(i, j) - u(i + 1, j) - u(i - 1, j) - L u(i, j + 1) - L u(i, j - 1) = N f(i, j)
			set(i, j, M);
			set(i + 1, j, Float(-1.0));
			set(i - 1, j, Float(-1.0));
			set(i, j + 1, -L);
			set(i, j - 1, -L);
			setAugmentedColumn(N * f(i, j));
			row++;
		}
		{
			// M u(n - 1, j) - u(n - 2, j) - L u(n - 1, j + 1) - L u(n - 1, j - 1) = N f(n - 1, j) + u(n, j)
			set(n - 1, j, M);
			set(n - 2, j, Float(-1.0));
			set(n - 1, j + 1, -L);
			set(n - 1, j - 1, -L);
			setAugmentedColumn(N * f(n - 1, j) + u(n, j));
			row++;
		}
	}
	{
		// M u(1, m - 1) - u(2, m - 1) - L u(1, m - 2) = N f(1, m - 1) + u(0, m - 1) + L u(1, m)
		set(1, m - 1, M);
		set(2, m - 1, Float(-1.0));
		set(1, m - 2, -L);
		setAugmentedColumn(N * f(1, m - 1) + u(0, m - 1) + L * u(1, m));
		row++;
	}
	for (i32 i = 2; i <= n - 2; i++) {
		// M u(i, m - 1) - u(i + 1, m - 1) - u(i - 1, m - 1) - L u(i, m - 2) = N f(i, m - 1) + L u(i, m)
		set(i, m - 1, M);
		set(i + 1, m - 1, Float(-1.0)); 
		set(i - 1, m - 1, Float(-1.0));
		set(i, m - 2, -L);
		setAugmentedColumn(N * f(i, m - 1) + L * u(i, m));
		row++;
	}
	{
		// M u(n - 1, m - 1) - u(n - 2, m - 1) - L u(n - 1, m - 2) = N f(n - 1, m - 1) + u(n, m - 1) + L u(n - 1, m)
		set(n - 1, m - 1, M);
		set(n - 2, m - 1, Float(-1.0));
		set(n - 1, m - 2, -L);
		setAugmentedColumn(N * f(n - 1, m - 1) + u(n, m - 1) + L * u(n - 1, m));
		row++;
	}

	const auto matrixCopy = matrix.clone();
	const auto augmentedColumnCopy = augmentedColumn.clone();
	auto result = solveGaussianElimination(matrix, augmentedColumn);
	if (!result.has_value()) {
		return;
	}

	for (i64 j = 1; j <= m - 1; j++) {
		for (i64 i = 1; i <= n - 1; i++) {
			u(i, j) = (*result)(0, gridIndexToVariableIndex(i, j));
		}
	}

	//const auto matrixCopy = matrix.clone();
	//const auto augmentedColumnCopy = augmentedColumn.clone();
	//auto result = Matrix<Float>::uninitialized(augmentedColumn.size());
	//solveGaussSeidel(matrixViewFromMatrix(matrix), matrixViewFromMatrix(augmentedColumn), matrixViewFromMatrix(result));

	//for (i64 j = 1; j <= m - 1; j++) {
	//	for (i64 i = 1; i <= n - 1; i++) {
	//		u(i, j) = result(0, gridIndexToVariableIndex(i, j));
	//	}
	//}

	/*matrixPrint(matrixCopy);
	matrixPrint(matrixTranspose(matrixCopy) - matrix);*/

	/*const auto test = matrixCopy * (*result);
	matrixPrint(test - augmentedColumnCopy);*/
}

//void steadyStateHeatEquationTest() {
//	PoissonEquationSolver solver;
//	auto u = Matrix<f32>::zero(5, 5);
//	auto f = Matrix<f32>::zero(5, 5);
//	const auto region = Aabb(Vec2(0.0f), Vec2(0.5f));
//	const auto regionSize = region.size();
//	for (i64 i = 0; i < u.sizeX(); i++) {
//		const auto x = region.min.x + (f32(regionSize.x) / (u.sizeX() - 1)) * i;
//		u(i, 0) = 200.0f * x;
//	}
//	for (i64 i = 0; i < u.sizeY(); i++) {
//		const auto y = region.min.y + (f32(regionSize.y) / f32(u.sizeY() - 1)) * (u.sizeY() - 1 - i);
//		u(u.sizeX() - 1, i) = 200.0f * y;
//	}
//	solver.solve(matrixViewFromConstMatrix(f), matrixViewFromMatrix(u), region);
//	matrixPrint(u);
//	/*
//	The correct output is
//	| 0    25   50    75 100 |
//	| 0 18.75 37.5 56.25  75 |
//	| 0  12.5   25  37.5  50 |
//	| 0  6.25 12.5 18.75  25 |
//	| 0     0    0     0   0 |
//	*/
//	// Technically what the solver does is discretize the problem and then try to solve the discretized problem exactly. The that comes from the discretization of the partial derivatives is based on the 4th partial derivatives of u. In this case the 4th derivatives are zero so if the discretized problem is solved exacly then it's also an exact solution to the original problem.
//	// The exact solution is 400xy
//
//	auto exactResult = Matrix<f32>::uninitialized(u.size());
//	for (i64 j = 0; j < u.sizeY(); j++) {
//		for (i64 i = 0; i < u.sizeX(); i++) {
//			const auto x = region.min.x + (f32(regionSize.x) / (u.sizeX() - 1)) * i;
//			const auto y = region.min.y + (f32(regionSize.y) / f32(u.sizeY() - 1)) * (u.sizeY() - 1 - j);
//			exactResult(i, j) = 400.0f * x * y;
//		}
//	}
//	matrixPrint(exactResult);
//}
//
//void poissonEquationTest() {
//	PoissonEquationSolver solver;
//	auto u = Matrix<f32>::zero(7, 6);
//	auto f = Matrix<f32>::zero(u.size());
//
//	const auto region = Aabb(Vec2(0.0f), Vec2(2.0f, 1.0f));
//	const auto regionSize = region.size();
//	for (i64 i = 0; i < u.sizeX(); i++) {
//		const auto x = region.min.x + (f32(regionSize.x) / (u.sizeX() - 1)) * i;
//		u(i, 0) = exp(1.0f) * x;
//		u(i, u.sizeY() - 1) = x;
//	}
//	for (i64 i = 0; i < u.sizeY(); i++) {
//		const auto y = region.min.y + (f32(regionSize.y) / f32(u.sizeY() - 1)) * (u.sizeY() - 1 - i);
//		u(u.sizeX() - 1, i) = 2.0f * exp(y);
//		u(0, i) = 0;
//	}
//
//	for (i64 j = 0; j < u.sizeY(); j++) {
//		for (i64 i = 0; i < u.sizeX(); i++) {
//			const auto x = region.min.x + (f32(regionSize.x) / (u.sizeX() - 1)) * i;
//			const auto y = region.min.y + (f32(regionSize.y) / f32(u.sizeY() - 1)) * (u.sizeY() - 1 - j);
//			f(i, j) = x * exp(y);
//		}
//	}
//
//	solver.solve(matrixViewFromConstMatrix(f), matrixViewFromMatrix(u), region);
//	matrixPrint(u);
//
//
//	auto exactResult = Matrix<f32>::uninitialized(u.size());
//	for (i64 j = 0; j < u.sizeY(); j++) {
//		for (i64 i = 0; i < u.sizeX(); i++) {
//			const auto x = region.min.x + (f32(regionSize.x) / (u.sizeX() - 1)) * i;
//			const auto y = region.min.y + (f32(regionSize.y) / f32(u.sizeY() - 1)) * (u.sizeY() - 1 - j);
//			exactResult(i, j) = x * exp(y);
//		}
//	}
//
//	matrixPrint(exactResult);
//	matrixPrint(exactResult - u);
//}