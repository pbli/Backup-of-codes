struct s_mympu {
	short acclR[3];//register
	float q[4];
	float accls[3];//sensor frame
	float acclw[3];//world frame
	float rpy[3];//angles
	float Drpy[3];

};
extern struct s_mympu mympu;

int mympu_open(unsigned int rate);
int mympu_update();

