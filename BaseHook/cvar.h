struct config_s {
	int bhop;
	int str, str_key;
	int light, light_value;
	int recoil, calc_recoil;
	int vector;
	
	int aim_active;
	int aim_smooth;
	int aim_x_stand;
	int aim_y_stand;
	int aim_z_stand;
	int aim_x_duck;
	int aim_y_duck;
	int aim_z_duck;
	int aim_x_jump;
	int aim_y_jump;
	int aim_z_jump;
};
extern config_s CFG[5];