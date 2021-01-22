#include "app.h"
#include "DD_Gene.h"
#include "DD_RCDefinition.h"
#include "SystemTaskManager.h"
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include "MW_GPIO.h"
#include "MW_IWDG.h"
#include "message.h"
#include "MW_flash.h"
#include "constManager.h"

/********/
/*メモ
 *g_ab_h...ABのハンドラ
 *g_md_h...MDのハンドラ
 *
 *g_rc_data...RCのデータ
 */
static
int bumper_setting(int num, bool oneshot, bool enable);
static
int bumper_medal_get(bool recet);

int appInit(void){

	ad_init();

	/*GPIO の設定などでMW,GPIOではHALを叩く*/
	return EXIT_SUCCESS;
}

/*application tasks*/
int appTask(void){
	
	int i;
	static win_medal = 0;

	if(MW_GPIORead(GPIOCID,GPIO_PIN_13) == 0){
		for(i=0;i<DD_NUM_OF_MD;i++){
			g_md_h[i].mode = D_MMOD_IN_GAME;
		}
		bumper_setting(4,false,true);
		win_medal += bumper_medal_get(false);
	}else{
		for(i=0;i<DD_NUM_OF_MD;i++){
			g_md_h[i].mode = D_MMOD_STANDBY;
		}
		bumper_setting(4,false,false);
		bumper_medal_get(true);
		win_medal = 0;
	}
	if( g_SY_system_counter % _MESSAGE_INTERVAL_MS < _INTERVAL_MS ){
		MW_printf("\n%d",win_medal);
	}
	return EXIT_SUCCESS;
}

static
int bumper_medal_get(bool recet){
	int recet_bit = 0;
	static int recent_read_bit = 0;
	static bool read_enable = true;
	int i;
	int count = 0;

	if(recet){
		read_enable = true;
		recent_read_bit = 0;
		return 0;
	}

	recet_bit = (g_md_h[PIC_TYPE1].rcv_data[1] >> 4) & 0b00000001;
	if(recet_bit == 0){
		g_md_h[PIC_TYPE1].snd_data[0] |= 0b00000001;
		g_md_h[PIC_TYPE1].snd_data[0] &= 0b11111101;
	}else if(recet_bit == 1){
		g_md_h[PIC_TYPE1].snd_data[0] |= 0b00000010;
		g_md_h[PIC_TYPE1].snd_data[0] &= 0b11111110;
	}
	if(recet_bit != recent_read_bit){
		read_enable = true;
	}
	if(read_enable){
		for(i=0;i<4;i++){
			if(((g_md_h[PIC_TYPE1].rcv_data[1] >> i) & 0b00000001) == 1) count++;
		}
		read_enable = false;
	}else{
		count = 0;
	}
	recent_read_bit = recet_bit;
	return count;
}

static
int bumper_setting(int num, bool oneshot, bool enable){
	if(!oneshot){
		if(num == 4){
			if(enable){
				g_md_h[PIC_TYPE1].snd_data[1] |= 0b11110000;
			}else{
				g_md_h[PIC_TYPE1].snd_data[1] &= 0b00001111;
			}
		}else{
			if(enable){
				g_md_h[PIC_TYPE1].snd_data[1] |= (0x01 << (num+4));
			}else{
				g_md_h[PIC_TYPE1].snd_data[1] &= (0b11111111 ^ (0x01 << (num+4)));
			}
		}
	}else{
		if(num == 4){
			if(enable){
				g_md_h[PIC_TYPE1].snd_data[1] &= 0b00001111;
				g_md_h[PIC_TYPE1].snd_data[1] |= 0b00001111;
			}else{
				g_md_h[PIC_TYPE1].snd_data[1] &= 0b11110000;
			}
		}else{
			if(enable){
				g_md_h[PIC_TYPE1].snd_data[1] &= (0b11111111 ^ (0x01 << (num+4)));
				g_md_h[PIC_TYPE1].snd_data[1] |= (0x01 << num);
			}else{
				g_md_h[PIC_TYPE1].snd_data[1] &= (0b11111111 ^ (0x01 << num));
			}
		}
	}
}
