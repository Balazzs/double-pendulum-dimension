#define NN 5
#define L 9.81
#define g 9.81
#define PI 3.14159265358979323846264

void derivs(__private double* x, __private double* f)
{
	const double t1 = x[1], t2 = x[3], p1 = x[2], p2 = x[4];
	const double COS = cos(t1 - t2);

	f[0] = 1;
	f[1] = 6.0 / (1.0 * L * L) * (2 * p1 - 3 * COS * p2) / (16 - 9 * COS*COS);
	f[3] = 6.0 / (1.0 * L * L) * (8 * p2 - 3 * COS * p1) / (16 - 9 * COS*COS);

	f[2] = -1.0 / 2 * 1 * L* L * ( f[1] * f[3] * sin(t1 - t2) + 3 * g / L * sin(t1));
	f[4] = -1.0 / 2 * 1 * L* L * (-f[1] * f[3] * sin(t1 - t2) + g / L * sin(t2));
}

void addTo(__private double* to, __private double* what, double szorzo)
{
	for (int i = 0; i < NN; i++)
		to[i] += what[i] * szorzo;
}

void multiply(__private double* from, __private double* to, double szorzo)
{
	for (int i = 0; i < NN; i++)
		to[i] = from[i] * szorzo;
}

//  Fourth order Runge-Kutta
void rk4(__private double* x, double tau, __private double* temp, __private double* deriv)
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

__kernel void load(__global double* states, int N)
{
	const size_t g_x = get_global_id(0), g_y = get_global_id(1);
	const size_t x = get_local_id(0), y = get_local_id(1);
	const size_t g_w = get_local_size(0), g_h = get_local_size(1);
	const size_t w = get_global_size(0), h = get_global_size(1);
	
	states[(g_y * N + g_x) * NN + 0] = 0;
	states[(g_y * N + g_x) * NN + 1] = -3.0 + 6.0 * g_x / N;
	states[(g_y * N + g_x) * NN + 2] = 0;
	states[(g_y * N + g_x) * NN + 3] = -3.0 + 6.0 * g_y / N;;
	states[(g_y * N + g_x) * NN + 4] = 0;
}

__kernel void prog(__global double* states, int N, double dt, int steps)
{
	const size_t g_x = get_global_id(0), g_y = get_global_id(1);
	const size_t x = get_local_id(0), y = get_local_id(1);
	const size_t g_w = get_local_size(0), g_h = get_local_size(1);
	const size_t w = get_global_size(0), h = get_global_size(1);
	
	double state[NN], temp[NN], deriv[NN];

	state[0] = states[(g_y * N + g_x) * NN + 0];
	state[1] = states[(g_y * N + g_x) * NN + 1];
	state[2] = states[(g_y * N + g_x) * NN + 2];
	state[3] = states[(g_y * N + g_x) * NN + 3];
	state[4] = states[(g_y * N + g_x) * NN + 4];


	//Alapból -1
	//times[g_y * N + g_x] = -1;
			
	for (int i = 0; i < steps && !(state[3] < -PI || state[3] > PI); i++)
	{
		rk4(state, dt, temp, deriv);
	}
	
	states[(g_y * N + g_x) * NN + 0] = state[0];
	states[(g_y * N + g_x) * NN + 1] = state[1];
	states[(g_y * N + g_x) * NN + 2] = state[2];
	states[(g_y * N + g_x) * NN + 3] = state[3];
	states[(g_y * N + g_x) * NN + 4] = state[4];
	/*
	while (state[0] < tMax)
	{
		rk4(state, dt, temp, deriv);
		if (state[3] < -PI || state[3] > PI)
			times[g_y * N + g_x] = state[0];
	}*/


	/*
	for(size_t i = 0; i < N; i++)
	{
		//Töltsük be õket a global memoryból, a megfelelõ blokkokat (A és B b-bõl)
		event_t betolt[2];
		betolt[0] = async_work_group_copy(A_buffer, A + (b_y * SX * SA + S*SA * i), SA * S, 0);
		betolt[1] = async_work_group_copy(B_buffer, B + (i * SX * SA + b_x * S*SB), S * SB, 0);
		//Várjuk meg amíg betöltõdnek
		wait_group_events(2, betolt);
		
		//Szummázuk le számonként (minden work item 1 szám)
		for(size_t j = 0; j < S; j++)
			C_buffer[y * SB + x] += A_buffer[y * S + j] * B_buffer[j * SB + x];
		
		//Nem kezdjük el a következõ másolást amíg nincs vége az összegezgetésnek (ez kell ide?)
		barrier(CLK_LOCAL_MEM_FENCE);
	}
	
	//És másoljuk ki az eredményt a global memoryba
	event_t kiir;
	kiir = async_work_group_copy(C + (b_y * SX * SA + SB*SA * b_x), C_buffer, SA*SB, 0);
	wait_group_events(1, &kiir);*/
}
