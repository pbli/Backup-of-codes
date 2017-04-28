#include "mpu.h"
#include "inv_mpu.h"
#include "inv_mpu_dmp_motion_driver.h"
#include "math.h"

#define FSR 2000 //if 250 will error and drift
#define QUAT_SENS       1073741824.f //2^30
#define ACCL_SENS       16384.0f
#define g               9800.0f //mm/s^2
#define PI_2            1.57079632679489661923f

struct s_mympu mympu;

struct s_quat { float w, x, y, z; }; 

union u_quat {
	struct s_quat _f;
	long _l[4];
} q;

static int ret;
static short accl[3];
static long accel_bias[3];
static short sensors;
static unsigned char fifoCount;

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
	accel_bias[0]=850;// accl calibration Pengbo li
	accel_bias[1]=2600;
	accel_bias[2]=-450;//4.5*delta
	ret=mpu_set_accel_bias(accel_bias);// the real value is in inv_mpu.c L1002 accel calibration is very important according to tests
	ret = mpu_set_dmp_state(1);
	ret = dmp_enable_feature(DMP_FEATURE_6X_LP_QUAT|DMP_FEATURE_SEND_RAW_ACCEL|DMP_FEATURE_GYRO_CAL);
	return 0;
}



int mympu_update(int i) {

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

	q._f.w = (float)q._l[0] / (float)QUAT_SENS;
	q._f.x = (float)q._l[1] / (float)QUAT_SENS;
	q._f.y = (float)q._l[2] / (float)QUAT_SENS;
	q._f.z = (float)q._l[3] / (float)QUAT_SENS;
	
	//normalized q
	mympu.q[0]=q._f.w;
	mympu.q[1]=q._f.x;
	mympu.q[2]=q._f.y;
	mympu.q[3]=q._f.z;
	
	
	//acc register value
	mympu.acclR[0] = accl[0] ;
	mympu.acclR[1] = accl[1] ;
	mympu.acclR[2] = accl[2] ;
	
	//acc sensor value with g	
	mympu.accls[0] = (float)accl[0] *g/ACCL_SENS;
	mympu.accls[1] = (float)accl[1] *g/ACCL_SENS;
	mympu.accls[2] = (float)accl[2] *g/ACCL_SENS;
	
	float q[4];
	q[0]=mympu.q[0];q[1]=mympu.q[1];q[2]=mympu.q[2];q[3]=mympu.q[3];
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
	mympu.roll  = atan2(_2q01+_2q23, 1-_2q11 -_2q22);
	mympu.pitch= asin(_2q02-_2q13);
	mympu.yaw  = atan2(_2q12+_2q03, 1-_2q33 -_2q22);
	
    mympu.yaw   *= 90.0f / PI_2; 
	mympu.pitch *= 90.0f / PI_2;	    
    mympu.roll  *= 90.0f / PI_2;
	//world frame acc    mympu.acclw[3*i+0]=(-(1-_2q22-_2q33)*(float)accl[0]-(_2q12-_2q03)*(float)accl[1]-(_2q13+_2q02)*(float)accl[2])*g/ACCL_SENS;
	mympu.acclw[3*i+1]=(-(_2q12+_2q03)*(float)accl[0]-(1-_2q11-_2q33)*(float)accl[1]-(_2q23-_2q01)*(float)accl[2])*g/ACCL_SENS;
	mympu.acclw[3*i+2]=(16384-(_2q13-_2q02)*(float)accl[0]-(_2q23+_2q01)*(float)accl[1]-(1-_2q11-_2q22)*(float)accl[2])*g/ACCL_SENS;
	/* need to adjust signs depending on the MPU mount orientation */ 
	return 0;
}

