#define NN 5
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
	for (int i = 0; i < NN; i++)
		to[i] += what[i] * szorzo;
}

void multiply(__private number* from, __private number* to, number szorzo)
{
	for (int i = 0; i < NN; i++)
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

__kernel void load(__global number* states, int N)
{
	const size_t g_x = get_global_id(0), g_y = get_global_id(1);
	const size_t x = get_local_id(0), y = get_local_id(1);
	
	states[(g_y * N + g_x) * NN + 0] = 0;
	states[(g_y * N + g_x) * NN + 1] = -3.0 + 6.0 * g_x / N;
	states[(g_y * N + g_x) * NN + 2] = 0;
	states[(g_y * N + g_x) * NN + 3] = -3.0 + 6.0 * g_y / N;;
	states[(g_y * N + g_x) * NN + 4] = 0;
}

__kernel void prog(__global number* states, int N, number dt, int steps)
{
	const size_t g_x = get_global_id(0), g_y = get_global_id(1);
	const size_t x = get_local_id(0), y = get_local_id(1);
	
	number state[NN], temp[NN], deriv[NN];

	state[0] = states[(g_y * N + g_x) * NN + 0];
	state[1] = states[(g_y * N + g_x) * NN + 1];
	state[2] = states[(g_y * N + g_x) * NN + 2];
	state[3] = states[(g_y * N + g_x) * NN + 3];
	state[4] = states[(g_y * N + g_x) * NN + 4];

	for (int i = 0; i < steps && !(state[3] < -PI || state[3] > PI); i++)
	{
		rk4(state, dt, temp, deriv);
	}
	
	states[(g_y * N + g_x) * NN + 0] = state[0];
	states[(g_y * N + g_x) * NN + 1] = state[1];
	states[(g_y * N + g_x) * NN + 2] = state[2];
	states[(g_y * N + g_x) * NN + 3] = state[3];
	states[(g_y * N + g_x) * NN + 4] = state[4];
}
