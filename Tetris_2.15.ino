#include "U8glib.h"
#include"MsTimer2.h"
#define LCD_BACKLIGHT_PIN 7
U8GLIB_PCD8544 u8g(2, 3, 5, 4, 6);

int block[10][20];                  /*二维数组存放绘图信息，为1画图，0空白，总共10*27个格子*/
int block_next[5][5];               /*存放下一个方块信息*/
int s = 1, life = 0;                   /*判断是否进行游戏*/
int x, y;                          /*方块中心点坐标*/
int x_next = 2 , y_next = 2;
int dir, score, block_num, block_num_next;  /*按键、分数值、方块形状编码、下一个方块形状编码）*/
int switchVoltage, move_speed, level_now, light_value;              /*读取按键的模拟值、下降速度、背光亮度*/
int block_state_right, block_state_left, block_state_down;  /*判断方块是否能够移动为0时可以移动*/

void setup()
{
  MsTimer2::set(100, Control);                /*定时器中断按键的扫描*/
  MsTimer2::start();
  Serial.begin(9600);
  randomSeed(analogRead(1));   /*随机产生随机数序列以免方块的顺序为固定的*/
  block_num = random(1, 20);
  block_num_next = random(1, 20) ;
  x = 5 , y = 2;
  block_state_right = 0 ;
  block_state_left = 0;
  block_state_down = 0;
  level_now = 300;
  light_value = 50;     /*设置背光亮度*/
  pinMode(LCD_BACKLIGHT_PIN , OUTPUT);
  analogWrite(LCD_BACKLIGHT_PIN, light_value);
}

void loop()
{
  analogWrite(LCD_BACKLIGHT_PIN, light_value);
  if ( life == 1)
    game_begin();
  else
    game_menu();
}

void draw_block()     /*绘制方块*/
{
  int block_x , block_y;
  for (block_x = 0 ; block_x < 10 ; block_x ++)
    for (block_y = 0 ; block_y < 20 ; block_y ++)
    {
      if (block[block_x][block_y] == 1)
        u8g.drawBox(2 * block_x + 28 , 2 * block_y + 4 , 2 , 2);
    }
}

void draw_frame()           /*绘制游戏边框*/
{
  int frame_x , frame_y ;
  for (frame_y = 0 ; frame_y <= 44 ; frame_y += 4)
  {
    u8g.drawFrame(24, frame_y, 4, 4);
    u8g.drawFrame(48, frame_y, 4, 4);
  }
  for (frame_x = 24 ; frame_x <= 48 ; frame_x += 4)
  {
    u8g.drawFrame(frame_x, 0, 4, 4);
    u8g.drawFrame(frame_x, 44, 4, 4);
  }
}

void game_begin()
{
  Serial.print(life);
  u8g.firstPage();
  do
  {
    draw_frame();   /*边框*/
    draw_block();
    interface();
  } while (u8g.nextPage());
  block_go();
  remove_block();
  move_speed = level_now - score; /*分数增加时下降速度随之增加*/
  if ( move_speed <= 100)
    move_speed = 100;
  delay(move_speed);
}

void block_go()             /*方块的移动和变形*/
{
  block_fixed();
  if (block_state_down == 1 )    /*方块不能下降的时候生成新的方块*/
  {
    for (int m = 0; m < 10; m++)
    {
      if (block[m][3] == 1) /*判断游戏是否结束*/
      {
        game_over();
        break;
      }
    }
    x = 5 , y = 2;    /*设置初始中心点位置*/
    block_num = block_num_next ;
    block_num_next = random(1, 20) ;
    block_state_right = 0 ;
    block_state_left = 0;
    block_state_down = 0;
    create_box(0);
  }

  if ( block_state_down == 0)  /*方块状态为0时持续下降*/
  {
    create_box(0);         /*清除上一个方块的信息，防止和新的方块产生影响*/
    y++;
    create_box(1);        /*更新中心点坐标后重新写入方块信息*/
  }
  /*判断是否能够移动*/
  switch (dir)
  {
    case 1:           /*变换方块形状*/
      create_box(0);
      if ( block_num >= 1 && block_num <= 4)   /*防止变换的时候方块形状发生变换*/
      {
        block_num++;
        if (block_num > 4)
          block_num = 1;
      }
      else if ( block_num >= 5 && block_num <= 8)
      {
        block_num++;
        if (block_num > 8)
          block_num = 5;
      }
      else if ( block_num >= 9 && block_num <= 12)
      {
        block_num++;
        if (block_num > 12)
          block_num = 9;
      }
      else if ( block_num >= 13 && block_num <= 14)
      {
        block_num++;
        if (block_num > 14)
          block_num = 13;
      }
      else if ( block_num >= 15 && block_num <= 16)
      {
        block_num++;
        if (block_num > 16)
          block_num = 15;
      }
      else if ( block_num >= 17 && block_num <= 18)
      {
        block_num++;
        if (block_num > 18)
          block_num = 17;
      }
      else if ( block_num == 19)
      {
        block_num = 19;
      }
      dir = 0;
      create_box(1);
      delay(50);
      break;
    case 2:          /*方块右移*/
      if ( block_state_right == 0)
      {
        create_box(0);   /*清空上个方块信息*/
        x++;
        create_box(1);
        dir  = 0;
      }
      delay(50);
      break;
    case 4:         /*方块左移*/
      if ( block_state_left == 0)
      {
        create_box(0);
        x--;
        create_box(1);
        dir  = 0;
      }
      delay(50);
      break;
  }
}

void create_box(int a )  /*调整block数组中的值，参数为0的时候清除，参数为1时写入*/
{
  switch (block_num)
  {
    case 1:
      block[x][y + 1] = a;
      block[x][y - 1] = a;
      block[x - 1][y] = a;
      block[x][y] = a;
      break ;
    case 2:
      block[x][y + 1] = a;
      block[x][y - 1] = a;
      block[x + 1][y] = a;
      block[x][y] = a;
      break;
    case 3:
      block[x + 1][y] = a;
      block[x - 1][y] = a;
      block[x][y + 1] = a;
      block[x][y] = a;
      break;
    case 4:
      block[x - 1][y] = a;
      block[x + 1][y] = a;
      block[x][y - 1] = a;
      block[x][y] = a;
      break;
    case 5:
      block[x][y] = a;
      block[x][y - 1] = a;
      block[x][y - 2] = a;
      block[x + 1][y] = a;
      break;
    case 6:
      block[x][y] = a;
      block[x + 1][y] = a;
      block[x + 2][y] = a;
      block[x][y + 1] = a;
      break;
    case 7:
      block[x][y] = a;
      block[x - 1][y] = a;
      block[x][y + 1] = a;
      block[x][y + 2] = a;
      break;
    case 8:
      block[x][y] = a;
      block[x][y - 1] = a;
      block[x - 1][y] = a;
      block[x - 2][y] = a;
      break;
    case 9:
      block[x][y] = a;
      block[x - 1][y] = a;
      block[x][y - 1] = a;
      block[x][y - 2] = a;
      break;
    case 10:
      block[x][y] = a;
      block[x][y - 1] = a;
      block[x + 1][y] = a;
      block[x + 2][y] = a;
      break;
    case 11:
      block[x][y] = a;
      block[x + 1][y] = a;
      block[x][y + 1] = a;
      block[x][y + 2] = a;
      break;
    case 12:
      block[x][y] = a;
      block[x][y + 1] = a;
      block[x - 1][y] = a;
      block[x - 2][y] = a;
      break;
    case 13:
      block[x][y + 2] = a;
      block[x][y + 1] = a;
      block[x][y - 1] = a;
      block[x][y] = a;
      break;
    case 14:
      block[x + 2][y] = a;
      block[x + 1][y] = a;
      block[x - 1][y] = a;
      block[x][y] = a;
      break;
    case 15:
      block[x + 1][y] = a;
      block[x][y + 1] = a;
      block[x + 1][y - 1] = a;
      block[x][y] = a;
      break;
    case 16:
      block[x + 1][y] = a;
      block[x - 1][y - 1] = a;
      block[x][y - 1] = a;
      block[x][y] = a;
      break;
    case 17:
      block[x - 1][y] = a;
      block[x][y + 1] = a;
      block[x - 1][y - 1] = a;
      block[x][y] = a;
      break;
    case 18:
      block[x + 1][y] = a;
      block[x][y + 1] = a;
      block[x - 1][y + 1] = a;
      block[x][y] = a;
      break;
    case 19:
      block[x + 1][y + 1] = a;
      block[x + 1][y] = a;
      block[x][y + 1] = a;
      block[x][y] = a;
      break;
  }
}

void create_box_next(int a) /*用于显示下一个方块，和上面的操作一样*/
{
  switch (block_num_next)
  {
    case 1:
      block_next[x_next][y_next + 1] = a;
      block_next[x_next][y_next - 1] = a;
      block_next[x_next - 1][y_next] = a;
      block_next[x_next][y_next] = a;
      break ;
    case 2:
      block_next[x_next][y_next + 1] = a;
      block_next[x_next][y_next - 1] = a;
      block_next[x_next + 1][y_next] = a;
      block_next[x_next][y_next] = a;
      break;
    case 3:
      block_next[x_next + 1][y_next] = a;
      block_next[x_next - 1][y_next] = a;
      block_next[x_next][y_next + 1] = a;
      block_next[x_next][y_next] = a;
      break;
    case 4:
      block_next[x_next - 1][y_next] = a;
      block_next[x_next + 1][y_next] = a;
      block_next[x_next][y_next - 1] = a;
      block_next[x_next][y_next] = a;
      break;
    case 5:
      block_next[x_next][y_next] = a;
      block_next[x_next][y_next - 1] = a;
      block_next[x_next][y_next - 2] = a;
      block_next[x_next + 1][y_next] = a;
      break;
    case 6:
      block_next[x_next][y_next] = a;
      block_next[x_next + 1][y_next] = a;
      block_next[x_next + 2][y_next] = a;
      block_next[x_next][y_next + 1] = a;
      break;
    case 7:
      block_next[x_next][y_next] = a;
      block_next[x_next - 1][y_next] = a;
      block_next[x_next][y_next + 1] = a;
      block_next[x_next][y_next + 2] = a;
      break;
    case 8:
      block_next[x_next][y_next] = a;
      block_next[x_next][y_next - 1] = a;
      block_next[x_next - 1][y_next] = a;
      block_next[x_next - 2][y_next] = a;
      break;
    case 9:
      block_next[x_next][y_next] = a;
      block_next[x_next - 1][y_next] = a;
      block_next[x_next][y_next - 1] = a;
      block_next[x_next][y_next - 2] = a;
      break;
    case 10:
      block_next[x_next][y_next] = a;
      block_next[x_next][y_next - 1] = a;
      block_next[x_next + 1][y_next] = a;
      block_next[x_next + 2][y_next] = a;
      break;
    case 11:
      block_next[x_next][y_next] = a;
      block_next[x_next + 1][y_next] = a;
      block_next[x_next][y_next + 1] = a;
      block_next[x_next][y_next + 2] = a;
      break;
    case 12:
      block_next[x_next][y_next] = a;
      block_next[x_next][y_next + 1] = a;
      block_next[x_next - 1][y_next] = a;
      block_next[x_next - 2][y_next] = a;
      break;
    case 13:
      block_next[x_next][y_next + 2] = a;
      block_next[x_next][y_next + 1] = a;
      block_next[x_next][y_next - 1] = a;
      block_next[x_next][y_next] = a;
      break;
    case 14:
      block_next[x_next + 2][y_next] = a;
      block_next[x_next + 1][y_next] = a;
      block_next[x_next - 1][y_next] = a;
      block_next[x_next][y_next] = a;
      break;
    case 15:
      block_next[x_next + 1][y_next] = a;
      block_next[x_next][y_next + 1] = a;
      block_next[x_next + 1][y_next - 1] = a;
      block_next[x_next][y_next] = a;
      break;
    case 16:
      block_next[x_next + 1][y_next] = a;
      block_next[x_next - 1][y_next - 1] = a;
      block_next[x_next][y_next - 1] = a;
      block_next[x_next][y_next] = a;
      break;
    case 17:
      block_next[x_next - 1][y_next] = a;
      block_next[x_next][y_next + 1] = a;
      block_next[x_next - 1][y_next - 1] = a;
      block_next[x_next][y_next] = a;
      break;
    case 18:
      block_next[x_next + 1][y_next] = a;
      block_next[x_next][y_next + 1] = a;
      block_next[x_next - 1][y_next + 1] = a;
      block_next[x_next][y_next] = a;
      break;
    case 19:
      block_next[x_next + 1][y_next + 1] = a;
      block_next[x_next + 1][y_next] = a;
      block_next[x_next][y_next + 1] = a;
      block_next[x_next][y_next] = a;
      break;
  }
}
void block_fixed()      /*判断方块是否能够移动*/
{
  switch (block_num)
  {
    case 1:
      if (block[x - 2][y] == 1 || block[x - 1][y - 1] == 1 || block[x - 1][y + 1] == 1 || x <= 1)
        block_state_left = 1;
      else
        block_state_left = 0;
      if (block[x + 1][y] == 1 || block[x + 1][y - 1] == 1 || block[x + 1][y + 1] == 1 || x >= 9)
        block_state_right = 1;
      else
        block_state_right = 0;
      if (block[x - 1][y + 1] == 1 || block[x][y + 2] == 1 || y >= 18)
        block_state_down = 1;
      else
        block_state_down = 0;
      break;
    case 2:
      if (block[x + 2][y] == 1 || block[x + 1][y - 1] == 1 || block[x + 1][y + 1] == 1 || x >= 8)
        block_state_right = 1;
      else
        block_state_right = 0;
      if (block[x - 1][y] == 1 || block[x - 1][y - 1] == 1 || block[x - 1][y + 1] == 1 || x <= 0)
        block_state_left = 1;
      else
        block_state_left = 0;
      if (block[x + 1][y + 1] == 1 || block[x][y + 2] == 1 || y >= 18)
        block_state_down = 1;
      else
        block_state_down = 0;
      break;
    case 3:
      if (block[x - 2][y] == 1 || block[x - 1][y + 1] == 1 || x <= 1)
        block_state_left = 1;
      else
        block_state_left = 0;
      if (block[x + 2][y] == 1 || block[x + 1][y + 1] == 1 || x >= 8)
        block_state_right = 1;
      else
        block_state_right = 0;
      if (block[x + 1][y + 1] == 1 || block[x - 1][y + 1] == 1 || block[x][y + 2] == 1 || y >= 18)
        block_state_down = 1;
      else
        block_state_down = 0;
      break;
    case 4:
      if (block[x - 2][y] == 1 || block[x - 1][y - 1] == 1 || x <= 1)
        block_state_left = 1;
      else
        block_state_left = 0;
      if (block[x + 2][y] == 1 || block[x + 1][y - 1] == 1 || x >= 8)
        block_state_right = 1;
      else
        block_state_right = 0;
      if (block[x][y + 1] == 1 || block[x - 1][y + 1] == 1 || block[x + 1][y + 1] == 1 || y >= 19)
        block_state_down = 1;
      else
        block_state_down = 0;
      break;
    case 5:
      if (block[x - 1][y] == 1 || block[x - 1][y - 1] == 1 || block[x - 1][y - 2] == 1 || x <= 0)
        block_state_left = 1;
      else
        block_state_left = 0;
      if (block[x + 2][y] == 1 || block[x + 1][y - 1] == 1 || block[x + 1][y - 2] == 1 || x >= 8)
        block_state_right = 1;
      else
        block_state_right = 0;
      if (block[x][y + 1] == 1 || block[x + 1][y + 1] == 1 || y >= 19)
        block_state_down = 1;
      else
        block_state_down = 0;
      break;
    case 6:
      if (block[x - 1][y] == 1 || block[x - 1][y + 1] == 1 || x <= 0)
        block_state_left = 1;
      else
        block_state_left = 0;
      if (block[x + 3][y] == 1 || block[x + 1][y + 1] == 1 || x >= 7)
        block_state_right = 1;
      else
        block_state_right = 0;
      if (block[x][y + 2] == 1 || block[x + 1][y + 1] == 1 || block[x + 2][y + 1] == 1 || y >= 18)
        block_state_down = 1;
      else
        block_state_down = 0;
      break;
    case 7:
      if (block[x - 2][y] == 1 || block[x - 1][y + 1] == 1 || block[x - 1][y + 2] == 1 || x <= 1)
        block_state_left = 1;
      else
        block_state_left = 0;
      if (block[x + 1][y] == 1 || block[x + 1][y + 1] == 1 || block[x + 1][y + 2] == 1 || x >= 9)
        block_state_right = 1;
      else
        block_state_right = 0;
      if (block[x - 1][y + 1] == 1 || block[x][y + 3] == 1 || y >= 17)
        block_state_down = 1;
      else
        block_state_down = 0;
      break;
    case 8:
      if (block[x - 1][y - 1] == 1 || block[x - 3][y] == 1 || x <= 2)
        block_state_left = 1;
      else
        block_state_left = 0;
      if (block[x + 1][y] == 1 || block[x + 1][y - 1] == 1 || x >= 9)
        block_state_right = 1;
      else
        block_state_right = 0;
      if (block[x][y + 1] == 1 || block[x - 1][y + 1] == 1 || block[x - 2][y + 1] == 1 || y >= 19)
        block_state_down = 1;
      else
        block_state_down = 0;
      break;
    case 9:
      if (block[x - 2][y] == 1 || block[x - 1][y - 1] == 1 || block[x - 1][y - 2] == 1 || x <= 1)
        block_state_left = 1;
      else
        block_state_left = 0;
      if (block[x + 1][y] == 1 || block[x + 1][y - 1] == 1 || block[x + 1][y - 2] == 1 || x >= 9)
        block_state_right = 1;
      else
        block_state_right = 0;
      if (block[x][y + 1] == 1 || block[x - 1][y + 1] == 1 || y >= 19)
        block_state_down = 1;
      else
        block_state_down = 0;
      break;
    case 10:
      if (block[x - 1][y] == 1 || block[x - 1][y - 1] == 1 || x <= 0)
        block_state_left = 1;
      else
        block_state_left = 0;
      if (block[x + 3][y] == 1 || block[x + 1][y - 1] == 1 || x >= 7)
        block_state_right = 1;
      else
        block_state_right = 0;
      if (block[x][y + 1] == 1 || block[x + 1][y + 1] == 1 || block[x + 2][y + 1] == 1 || y >= 19)
        block_state_down = 1;
      else
        block_state_down = 0;
      break;
    case 11:
      if (block[x - 1][y] == 1 || block[x - 1][y + 1] == 1 || block[x - 1][y + 2] == 1 || x <= 0)
        block_state_left = 1;
      else
        block_state_left = 0;
      if (block[x + 2][y] == 1 || block[x + 1][y + 1] == 1 || block[x + 1][y + 2] == 1 || x >= 8)
        block_state_right = 1;
      else
        block_state_right = 0;
      if (block[x][y + 3] == 1 || block[x + 1][y + 1] == 1 || y >= 17)
        block_state_down = 1;
      else
        block_state_down = 0;
      break;
    case 12:
      if (block[x - 3][y] == 1 || block[x - 1][y + 1] == 1 || x <= 2)
        block_state_left = 1;
      else
        block_state_left = 0;
      if (block[x + 1][y] == 1 || block[x + 1][y + 1] == 1 || x >= 9)
        block_state_right = 1;
      else
        block_state_right = 0;
      if (block[x][y + 2] == 1 || block[x - 1][y + 1] == 1 || block[x - 2][y + 1] == 1 || y >= 18)
        block_state_down = 1;
      else
        block_state_down = 0;
      break;
    case 13:
      if (block[x - 1][y - 1] == 1 || block[x - 1][y] == 1 || block[x - 1][y + 1] == 1 || block[x - 1][y + 2] == 1 || x <= 0)
        block_state_left = 1;
      else
        block_state_left = 0;
      if (block[x + 1][y - 1] == 1 || block[x + 1][y] == 1 || block[x + 1][y + 1] == 1 || block[x + 1][y + 2] == 1 || x >= 9)
        block_state_right = 1;
      else
        block_state_right = 0;
      if (block[x][y + 3] == 1 || y >= 17)
        block_state_down = 1;
      else
        block_state_down = 0;
      break;
    case 14:
      if (block[x - 2][y] == 1 || x <= 1)
        block_state_left = 1;
      else
        block_state_left = 0;
      if (block[x + 3][y] == 1 || x >= 7)
        block_state_right = 1;
      else
        block_state_right = 0;
      if (block[x - 1][y + 1] == 1 || block[x][y + 1] == 1 || block[x + 1][y + 1] == 1 || block[x + 2][y + 1] == 1 || y >= 19)
        block_state_down = 1;
      else
        block_state_down = 0;
      break;
    case 15:
      if (block[x - 1][y] == 1 || block[x - 1][y + 1] == 1 || block[x][y - 1] == 1 || x <= 0)
        block_state_left = 1;
      else
        block_state_left = 0;
      if (block[x + 2][y] == 1 || block[x + 1][y + 1] == 1 || block[x + 2][y - 1] == 1 || x >= 8)
        block_state_right = 1;
      else
        block_state_right = 0;
      if (block[x][y + 2] == 1 || block[x + 1][y + 1] == 1 || y >= 18)
        block_state_down = 1;
      else
        block_state_down = 0;
      break;
    case 16:
      if (block[x - 2][y - 1] == 1 || block[x - 1][y] == 1 || x <= 1)
        block_state_left = 1;
      else
        block_state_left = 0;
      if (block[x + 2][y] == 1 || block[x + 1][y - 1] == 1 || x >= 8)
        block_state_right = 1;
      else
        block_state_right = 0;
      if (block[x - 1][y] == 1 || block[x][y + 1] == 1 || block[x + 1][y + 1] == 1 || y >= 19)
        block_state_down = 1;
      else
        block_state_down = 0;
      break;
    case 17:
      if (block[x - 2][y - 1] == 1 || block[x - 2][y] == 1 || block[x - 1][y + 1] == 1 || x <= 1)
        block_state_left = 1;
      else
        block_state_left = 0;
      if (block[x + 1][y] == 1 || block[x][y - 1] == 1 || block[x + 1][y + 1] == 1 || x >= 9)
        block_state_right = 1;
      else
        block_state_right = 0;
      if (block[x - 1][y + 1] == 1 || block[x][y + 2] == 1 || y >= 18)
        block_state_down = 1;
      else
        block_state_down = 0;
      break;
    case 18:
      if (block[x - 1][y] == 1 || block[x - 2][y + 1] == 1 || x <= 1)
        block_state_left = 1;
      else
        block_state_left = 0;
      if (block[x + 2][y] == 1 || block[x + 1][y + 1] == 1 || x >= 8)
        block_state_right = 1;
      else
        block_state_right = 0;
      if (block[x][y + 2] == 1 || block[x - 1][y + 2] == 1 || block[x + 1][y + 1] == 1 || y >= 18)
        block_state_down = 1;
      else
        block_state_down = 0;
      break;
    case 19:
      if (block[x - 1][y] == 1 || block[x - 1][y + 1] == 1 || x <= 0)
        block_state_left = 1;
      else
        block_state_left = 0;
      if (block[x + 2][y] == 1 || block[x + 2][y + 1] == 1 || x >= 8)
        block_state_right = 1;
      else
        block_state_right = 0;
      if (block[x][y + 2] == 1 || block[x + 1][y + 2] == 1 || y >= 18)
        block_state_down = 1;
      else
        block_state_down = 0;
      break;
  }
}

void remove_block()
{
  int sum = 0, m, n, i = 0 , h;       /*i：需要消除的行数，h:记录是哪一行需要消除*/
  /*判断有多少行需要消除*/
  for ( m = 19 ; m > 4; m--)
  {
    for ( n = 0 ; n < 10 ; n++)
      sum += block[n][m];
    if (sum == 10)
    {
      i++;
      score += 10;
      h = m;
      block_state_down = 1 ;
    }
    else
      sum = 0;
  }
  /*当存在消行的情况下数组每一行都向下移动i个单位*/
  for (i; i > 0; i--)
    for ( m = h; m >= 3; m--)
      for ( n = 0; n < 10; n++)
        block[n][m] = block[n][m - 1];
}


void game_over()
{
  u8g.firstPage();
  do {
    u8g.setFont(u8g_font_courB10);
    u8g.drawStr(7, 15, "Gameover");
    u8g.setFont(u8g_font_ncenR08);
    u8g.drawStr(18, 28, "Your Score");
    u8g.setFont(u8g_font_fub11);
    u8g.setPrintPos(30, 45);
    u8g.print(score);
  } while (u8g.nextPage());
  delay(1500);
  /*清空数组*/
  for (int m = 0 ; m < 20 ; m++)
    for (int n = 0 ; n < 10 ; n++)
      block[n][m] = 0;
  life = 0;
}

void game_menu()
{
  u8g.firstPage();
  do {
    u8g.setFont(u8g_font_ncenB14);
    u8g.drawStr( 14, 16, "Tetris");
    u8g.setFont(u8g_font_courR08);
    u8g.drawStr( 5, 26 , "Begin");
    u8g.drawStr( 5, 36, "Level");
    u8g.drawStr( 5, 46, "Light");
    u8g.drawTriangle(0, 26 + 10 * (s - 1), 0, 20 + 10 * (s - 1), 3, 23 + 10 * (s - 1));
    u8g.drawTriangle(40, 26 + 10 * (s - 1), 40, 20 + 10 * (s - 1), 37, 23 + 10 * (s - 1));
    u8g.drawFrame(45, 22, 35, 25);
    if ( s == 1 )
    {
      u8g.drawBox(60, 27, 4, 12);
      u8g.drawBox(64, 35, 4, 4);
    }

    if ( s == 2)
    {
      if ( level_now <= 300)
      {
        u8g.drawBox(50, 39, 3, 6);
        u8g.drawFrame(55, 36, 3, 9);
        u8g.drawFrame(60, 33, 3, 12);
        u8g.drawFrame(65, 30, 3, 15);
        u8g.drawFrame(70, 27, 3, 18);
      }
      if ( level_now <= 250)
      {
        u8g.drawBox(50, 39, 3, 6);
        u8g.drawBox(55, 36, 3, 9);
        u8g.drawFrame(60, 33, 3, 12);
        u8g.drawFrame(65, 30, 3, 15);
        u8g.drawFrame(70, 27, 3, 18);
      }
      if ( level_now <= 200)
      {
        u8g.drawBox(50, 39, 3, 6);
        u8g.drawBox(55, 36, 3, 9);
        u8g.drawBox(60, 33, 3, 12);
        u8g.drawFrame(65, 30, 3, 15);
        u8g.drawFrame(70, 27, 3, 18);
      }
      if ( level_now <= 150)
      {
        u8g.drawBox(50, 39, 3, 6);
        u8g.drawBox(55, 36, 3, 9);
        u8g.drawBox(60, 33, 3, 12);
        u8g.drawBox(65, 30, 3, 15);
        u8g.drawFrame(70, 27, 3, 18);
      }
      if ( level_now <= 100)
      {
        u8g.drawBox(50, 39, 3, 6);
        u8g.drawBox(55, 36, 3, 9);
        u8g.drawBox(60, 33, 3, 12);
        u8g.drawBox(65, 30, 3, 15);
        u8g.drawBox(70, 27, 3, 18);
      }
    }

    if ( s == 3)
    {
      u8g.drawFrame(50, 32, 25, 4);
      u8g.drawFrame(50 + light_value / 10, 31, 4, 6);
      u8g.drawBox(50, 32, light_value / 10, 4);
    }

  } while (u8g.nextPage());
  if ( dir == 3)
  {
    s++;
    if (s > 3)
      s = 1;
    dir = 0;
  }
  if (dir == 1)
  {
    s--;
    if (s < 1)
      s = 3;
    dir = 0;
  }
  if ( s == 1 && dir == 5)
  {
    life = 1;
    dir = 0;
  }
  if ( s == 2 && dir == 2)
  {
    level_now -= 50;
    if (level_now < 100)
      level_now = 100;
    dir = 0;
  }
  if ( s == 2 && dir == 4)
  {
    level_now += 50;
    if (level_now >= 300)
      level_now = 300;
    dir = 0;
  }
  if ( s == 3 && dir == 2 )
  {
    light_value += 10;
    if (light_value >= 255)
      light_value = 255;
    dir = 0;
  }
  if ( s == 3 && dir == 4)
  {
    light_value -= 10;
    if (light_value <= 0)
      light_value = 0;
    dir = 0;
  }
}

void interface()
{
  u8g.setFont(u8g_font_timR08);
  u8g.drawStr(57, 10, "Score");
  u8g.drawStr(57, 30, "Grade");
  u8g.drawStr(0, 10, "Next");
  u8g.setPrintPos(70, 20);
  /*显示分数*/
  u8g.print(score);
  /*显示等级*/
  if ( move_speed <= 300)
    u8g.drawBox(60, 42, 3, 6);
  if ( move_speed <= 250)
    u8g.drawBox(65, 39, 3, 9);
  if ( move_speed <= 200)
    u8g.drawBox(70, 36, 3, 12);
  if ( move_speed <= 150)
    u8g.drawBox(75, 33, 3, 15);
  if (move_speed <= 100)
    u8g.drawBox(80, 30, 3, 18);
  /*显示下一个方块*/
  create_box_next(1);
  for (int m = 0 ; m < 5 ; m++)
    for (int n = 0 ; n < 5 ; n++)
    {
      if ( block_next[m][n] == 1)
        u8g.drawBox(3 * m + 5, 3 * n + 15, 3, 3);
    }
  create_box_next(0);
}

void Control()
{
  switchVoltage = analogRead(0);
  if ( switchVoltage > 600 && switchVoltage < 800 )            //上
    dir = 1 ;
  else if ( switchVoltage > 180 && switchVoltage < 400 )       //下
    dir = 3;
  else if ( switchVoltage == 0 )                               //左
    dir = 4;
  else if ( switchVoltage > 400 && switchVoltage < 600 )       //右
    dir = 2;
  else if ( switchVoltage > 0 && switchVoltage < 180 )        //确认
    dir = 5;
}
