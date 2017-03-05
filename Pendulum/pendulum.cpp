#include <cmath>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <string>
using namespace std;

#include <future>

#include "vector.hpp"        // vectors with components of type double
#include "odeint.hpp"        // ODE integration routines, Runge-Kutta ...
using namespace cpl;

const double pi = 4 * atan(1.0);

const double g = 9.8;        // acceleration of gravity

double L = 1.0;              // length of pendulum
double q = 0.5;              // damping coefficient
double Omega_D = 2.0 / 3.0;    // frequency of driving force
double F_D = 0.9;            // amplitude of driving force
bool nonlinear;              // linear if false

Vector f(const Vector& x) {  // extended derivative vector
	double t = x[0];
	double theta = x[1];
	double omega = x[2];
	Vector f(3);             // Vector with 3 components
	f[0] = 1;
	f[1] = omega;
	if (nonlinear)
		f[2] = -(g / L) * sin(theta) - q * omega + F_D * sin(Omega_D * t);
	else
		f[2] = -(g / L) * theta - q * omega + F_D * sin(Omega_D * t);
	return f;
}


Vector dp(const Vector& x)
{
	const double t = x[0];
	const double theta1 = x[1];
	const double p1 = x[2];
	const double theta2 = x[3];
	const double p2 = x[4];

	const double COS = cos(theta1 - theta2);

	//double p1 = 1.0 / 6.0 * 1.0 * L * L * (8 * omega1 + 3 * omega2 * COS);
	//double p2 = 1.0 / 6.0 * 1.0 * L * L * (2 * omega2 + 3 * omega1 * COS);

	Vector f(5);
	f[0] = 1;
	f[1] = 6.0 / (1.0 * L * L) * (2 * p1 - 3 * COS * p2) / (16 - 9 * COS*COS);
	f[3] = 6.0 / (1.0 * L * L) * (8 * p2 - 3 * COS * p1) / (16 - 9 * COS*COS);
	
	f[2] = -0.5 * 1 * L* L * (f[1] * f[3] * sin(theta1 - theta2) + 3 * g / L * sin(theta1) );
	f[4] = -0.5 * 1 * L* L * (-f[1] * f[3] * sin(theta1 - theta2) + g / L * sin(theta2) );

	return f;
}


int main_() {
	nonlinear = true;
	cout << " Length of pendulum L: ";
	cin >> L;/*
	cout << " Enter damping coefficient q: ";
	cin >> q;*/
	//cout << " Enter driving frequencey Omega_D: ";
	//cin >> Omega_D;
	Omega_D = 0.0;
	//cout << " Enter driving amplitude F_D: ";
	//cin >> F_D;
	F_D = 0.0;
	//cout << " Enter theta(0) and omega(0): ";
	//double theta, omega, tMax;
	//cin >> theta >> omega;
	double tMax;
	cout << " Enter integration time t_max: ";
	cin >> tMax;

	
	double dt = 0.05;
	double accuracy = 1e-6;
	ofstream dataFile("pendulum.data");
	/*
	///////////////////////////////////////
	ofstream file("12.data");
	Vector xx(5);
	xx[0] = 0;
	xx[1] = 3;
	xx[2] = 0.0;
	xx[3] = 3;
	xx[4] = 0.0;
	
	while (xx[0] < tMax) {

		adaptiveRK4Step(xx, dt, accuracy, dp);
		file << xx[0] << "\t" << xx[1] << "\t" << xx[3] << std::endl;
	}
	file.close();
	///////////////////////////////////////////
	//double theta, omega;
	return 0;*/
	const int N = 100;
	const double dTheta = 6.0 / N;
	vector< future < vector <double> > > fs(N);

	for (int i = 0; i < N; i++)
	{
		double theta_0 = -3.0 + i * dTheta;

		fs[i] = async([](const int N, const double theta_0, const double tMax) {

			vector<double> ret(N);

			double dt = 0.05;
			double accuracy = 1e-6;

			const double dTheta = 6.0 / N;
			for (int j = 0; j < N; j++)
			{
				double theta_1 = -3.0 + j * dTheta;
				double t = 0;

				Vector x(5);
				x[0] = t;
				x[1] = theta_0;
				x[2] = 0.0;
				x[3] = theta_1;
				x[4] = 0.0;

				while (x[0] < tMax) {
					adaptiveRK4Step(x, dt, accuracy, dp);
					//t = x[0], theta = x[1], omega = x[2];
					
					if (x[3] >= pi || x[3] < -pi)
					{
						ret[j] = x[0];
						break;
					}

				}
				if (x[0] >= tMax)
					ret[j] = tMax+1;

			}

			return ret;
		}, N, theta_0, tMax);
	}

	vector<vector<double>> vecs(N);
	for (int i = 0; i < N; i++)
		vecs[i] = fs[i].get();

	for (int i = 0; i < N; i++)
	{
		for (int j = 0; j < N; j++)
			dataFile << vecs[i][j] << '\t';
		dataFile << endl;
	}

	/*
	{
		
		for (double theta_1 = -3.0; theta_1 < 3.0; theta_1 += dTheta)
		{
			double t = 0;
			Vector x(5);
			x[0] = t;
			x[1] = theta_0;
			x[2] = 0.0;
			x[3] = theta_1;
			x[4] = 0.0;

			while (x[0] < tMax) {
				adaptiveRK4Step(x, dt, accuracy, dp);
				//t = x[0], theta = x[1], omega = x[2];

				if (x[3] >= pi || x[3] < -pi)
				{
					dataFile << x[0] << '\t';// << theta << '\t' << omega << '\n';
					break;
				}
				
			}
			if (x[0] >= tMax)
				dataFile << x[0] << '\t';
			
		}
		dataFile << endl;
	}*/
	cout << " Output data to file pendulum.data" << endl;
	dataFile.close();
	return 0;
}
