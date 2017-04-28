#include "mpu.h"
#include "inv_mpu.h"
#include "inv_mpu_dmp_motion_driver.h"
#include "math.h"

#define FSR				2000 //if 250 will error and drift
#define QUAT_SENS       1073741824.f //2^30
#define ACCL_SENS       16384.0f
#define g               9800.0f //mm/s^2
#define PI_2            1.570796f


struct s_mympu mympu;

struct s_quat { float w, x, y, z; }; 

union u_quat {
	struct s_quat _f;
	long _l[4];
} q;

static int ret;
static short accl[3];
static short sensors;
static unsigned char fifoCount;
//static float rpy[3]={0,90,0};



int mympu_open(unsigned int rate) {
  	mpu_select_device(0);
   	mpu_init_structures();
	ret = mpu_init(NULL);
	ret = mpu_set_sensors(INV_XYZ_GYRO|INV_XYZ_ACCEL);
	ret = mpu_set_gyro_fsr(FSR);
	ret = mpu_set_accel_fsr(2);
	mpu_get_power_state((unsigned char *)&ret);
	ret = mpu_configure_fifo(INV_XYZ_GYRO|INV_XYZ_ACCEL);
	dmp_select_device(0);
	dmp_init_structures();
	ret = dmp_load_motion_driver_firmware();
	ret = dmp_set_fifo_rate(rate);
	//make sure the stable noise of real acc small enough, then good.
	short accel_bias[3];
	accel_bias[0]=-200;//different sensor different values, can be more precise!
	accel_bias[1]=600;
	accel_bias[2]=200;
	ret=mpu_set_accel_bias(accel_bias);// the real value is in inv_mpu.c L1002 accel calibration is very important according to tests
	ret = mpu_set_dmp_state(1);
	ret = dmp_enable_feature(DMP_FEATURE_6X_LP_QUAT|DMP_FEATURE_SEND_RAW_ACCEL|DMP_FEATURE_GYRO_CAL);
	return 0;
}

int mympu_update( ) {

	do {
		ret = dmp_read_fifo(NULL,accl,q._l,NULL,&sensors,&fifoCount);
		/* will return:
			0 - if ok
			1 - no packet available
			2 - if BIT_FIFO_OVERFLOWN is set
			3 - if frame corrupted
		       <0 - if error
		*/

		if (ret!=0) return ret; 
	} while (fifoCount>1);

	//q
	mympu.q[0]= (float)q._l[0]/ (float)QUAT_SENS;
	mympu.q[1]= (float)q._l[1]/ (float)QUAT_SENS;
	mympu.q[2]= (float)q._l[2]/ (float)QUAT_SENS;
	mympu.q[3]= (float)q._l[3]/ (float)QUAT_SENS;
    //normalized q

    /*
	q._f.w = mympu.q[0] 
	q._f.x = mympu.q[1] 
	q._f.y = mympu.q[2]
	q._f.z = mympu.q[3] 
	


	
	//acc register value
	mympu.acclR[0] = accl[0] ;
	mympu.acclR[1] = accl[1] ;
	mympu.acclR[2] = accl[2] ;
	
	//acc sensor value with g
	mympu.accls[0] = (float)accl[0] *g/ACCL_SENS;
	mympu.accls[1] = (float)accl[1] *g/ACCL_SENS;
	mympu.accls[2] = (float)accl[2] *g/ACCL_SENS;

	float q[4];
	q[0]=q._f.w;q[1]=q._f.x;q[2]=q._f.y;q[3]=q._f.z;
	float _2q11=2*q[1]*q[1];
	float _2q22=2*q[2]*q[2];
	float _2q33=2*q[3]*q[3];
	float  _2q01=2*q[0]*q[1];
	float  _2q02=2*q[0]*q[2];
	float  _2q03=2*q[0]*q[3];
	float  _2q12=2*q[1]*q[2];
	float  _2q13=2*q[1]*q[3];
	float  _2q23=2*q[2]*q[3];
	
	//pitch roll yaw
	mympu.rpy[0]  = atan2((_2q01+_2q23),(1-_2q11 -_2q22));
    mympu.rpy[1]= asin(_2q02-_2q13);
	mympu.rpy[2]  = atan2((_2q03+_2q12),(1-_2q33 -_2q22));

	for (int j=0;j<3;j++)
	{
	mympu.rpy[j]   *= 90.0f / PI_2;
	}

   if (accl[2]>0)
    { 
	   mympu.rpy[1]=90-mympu.rpy[1];
    }
	if (accl[2]<0 )
    {
	   mympu.rpy[1]=mympu.rpy[1]+270;
    }

    for (int j=1;j<2;j++)
    {	
		float test=0;
		test=(int)(mympu.rpy[j]-rpy[j]);
		
		if (abs(test)>300)//jump
		{
			mympu.Drpy[j]+=test/abs(test)*360;
		}
		else if (abs(test)>130&&abs(test)<230)
		{
			mympu.Drpy[j]+=test/abs(test)*180;
		}

		rpy[j]=mympu.rpy[j];//keep track of last measurement
   }
    
	// after calibrate angles, then use static calibration to get scale, can be more precise
	accl[0]=accl[0]*0.99f;
	accl[1]=accl[1]*0.99f;
	accl[2]=accl[2]/1.018f;
	
	//world frame acc, wikipeadia has the formula, it is interia force, so add negtive    mympu.acclw[0]=(-(1-_2q22-_2q33)*(float)accl[0]-(_2q12-_2q03)*(float)accl[1]-(_2q13+_2q02)*(float)accl[2])*g/ACCL_SENS;
	mympu.acclw[1]=(-(_2q12+_2q03)*(float)accl[0]-(1-_2q11-_2q33)*(float)accl[1]-(_2q23-_2q01)*(float)accl[2])*g/ACCL_SENS;
	mympu.acclw[2]=9800+(-(_2q13-_2q02)*(float)accl[0]-(_2q23+_2q01)*(float)accl[1]-(1-_2q11-_2q22)*(float)accl[2])*g/ACCL_SENS;
	
	//sensor frame
	
	mympu.acclw[0]=(accl[0]-(_2q13-_2q02)*16384)*g/ACCL_SENS;
	mympu.acclw[1]=(accl[1]-(_2q23+_2q01)*16384)*g/ACCL_SENS;
	mympu.acclw[2]=((float)accl[2]-(1-_2q11-_2q22)*16384)*g/ACCL_SENS;
		
    */
	return 0;
}

