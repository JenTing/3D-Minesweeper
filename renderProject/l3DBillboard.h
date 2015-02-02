void l3dBillboardGetRightVector(float *right);
void l3dBillboardGetUpRightVector(float *up, float *right);
void l3dBillboardLocalToWorld(float *cam, float *worldPos);
void l3dBillboardCylindricalBegin(float *cam, float *worldPos);
void l3dBillboardSphericalBegin(float *cam, float *worldPos);
void l3dBillboardCheatSphericalBegin(void);
void l3dBillboardCheatCylindricalBegin(void);
void l3dBillboardEnd(void);
