#include "midia5.h"

void * _midia5_init_output_platform_data(MIDIA5_OUTPUT_HANDLE * hp, int device);
void _midia5_free_output_platform_data(MIDIA5_OUTPUT_HANDLE * hp);
void _midia5_platform_send_data(MIDIA5_OUTPUT_HANDLE * hp, int data);
void _midia5_platform_reset_output_device(MIDIA5_OUTPUT_HANDLE * hp);
bool _midia5_platform_set_output_gain(MIDIA5_OUTPUT_HANDLE * hp, float gain);

MIDIA5_OUTPUT_HANDLE * midia5_create_output_handle(int device)
{
	MIDIA5_OUTPUT_HANDLE * hp;

	hp = malloc(sizeof(MIDIA5_OUTPUT_HANDLE));
	if(hp)
	{
		hp->platform_data = _midia5_init_output_platform_data(hp, device);
		return hp;
	}
	return NULL;
}

void midia5_destroy_output_handle(MIDIA5_OUTPUT_HANDLE * hp)
{
	_midia5_free_output_platform_data(hp);
	free(hp);
}

void midia5_send_data(MIDIA5_OUTPUT_HANDLE * hp, int data)
{
	_midia5_platform_send_data(hp, data);
}

void midia5_reset_output_device(MIDIA5_OUTPUT_HANDLE * hp)
{
	_midia5_platform_reset_output_device(hp);
}

bool midia5_set_output_gain(MIDIA5_OUTPUT_HANDLE * hp, float gain)
{
	return _midia5_platform_set_output_gain(hp, gain);
}
