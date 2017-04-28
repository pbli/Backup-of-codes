struct s_mympu {
	short acclR[3];
	float q[4];
	float accls[3];
	float acclw[15];
	float yaw,pitch,roll;

};
extern struct s_mympu mympu;

int mympu_open(unsigned int rate);
int mympu_update();

