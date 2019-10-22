#define NUMBER_OF_VARIABLES 5
#define L 9.81
#define g 9.81
#define PI 3.14159265358979323846264

typedef float number;

void derivs(__private number* x, __private number* f)
{
	const number t1 = x[1], t2 = x[3], p1 = x[2], p2 = x[4];
	const number COS = cos(t1 - t2);

	f[0] = 1;
	f[1] = 6.0 / (1.0 * L * L) * (2 * p1 - 3 * COS * p2) / (16 - 9 * COS*COS);
	f[3] = 6.0 / (1.0 * L * L) * (8 * p2 - 3 * COS * p1) / (16 - 9 * COS*COS);

	f[2] = -1.0 / 2 * 1 * L* L * ( f[1] * f[3] * sin(t1 - t2) + 3 * g / L * sin(t1));
	f[4] = -1.0 / 2 * 1 * L* L * (-f[1] * f[3] * sin(t1 - t2) + g / L * sin(t2));
}

void addTo(__private number* to, __private number* what, number szorzo)
{
	for (int i = 0; i < NUMBER_OF_VARIABLES; i++)
		to[i] += what[i] * szorzo;
}

void multiply(__private number* from, __private number* to, number szorzo)
{
	for (int i = 0; i < NUMBER_OF_VARIABLES; i++)
		to[i] = from[i] * szorzo;
}

//  Fourth order Runge-Kutta
void rk4(__private number* x, number tau, __private number* temp, __private number* deriv)
{
	derivs(x, deriv);

	multiply(deriv, temp, tau);
	//temp - k1
	
	addTo(x, temp, 1.0/6.0);
	//+ 	

	multiply(temp, temp, 0.5);
	addTo(temp, x, 1.0);
	derivs(temp, temp);
	multiply(temp, temp, tau);
	//temp - k2

	addTo(x, temp, 2.0 / 6.0);
	//+

	multiply(temp, temp, 0.5);
	addTo(temp, x, 1.0);
	derivs(temp, temp);
	multiply(temp, temp, tau);
	//temp - k3

	addTo(x, temp, 2.0 / 6.0);
	//+

	addTo(temp, x, 1.0);
	derivs(temp, temp);
	multiply(temp, temp, tau);
	//temp - k4

	addTo(x, temp, 1.0 / 6.0);
	//+

	//voila kész a lépés
}

__kernel void prog(__global number* states, number dt, int steps)
{
	const size_t g_x = get_global_id (0);
	
	number state[NUMBER_OF_VARIABLES];
	number temp [NUMBER_OF_VARIABLES];
	number deriv[NUMBER_OF_VARIABLES];

	state[0] = states[g_x * NUMBER_OF_VARIABLES + 0];
	state[1] = states[g_x * NUMBER_OF_VARIABLES + 1];
	state[2] = states[g_x * NUMBER_OF_VARIABLES + 2];
	state[3] = states[g_x * NUMBER_OF_VARIABLES + 3];
	state[4] = states[g_x * NUMBER_OF_VARIABLES + 4];

	for (int i = 0; i < steps && !(state[3] < -PI || state[3] > PI); i++) {
		rk4(state, dt, temp, deriv);
	}
	
	states[g_x * NUMBER_OF_VARIABLES + 0] = state[0];
	states[g_x * NUMBER_OF_VARIABLES + 1] = state[1];
	states[g_x * NUMBER_OF_VARIABLES + 2] = state[2];
	states[g_x * NUMBER_OF_VARIABLES + 3] = state[3];
	states[g_x * NUMBER_OF_VARIABLES + 4] = state[4];
}
