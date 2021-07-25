#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <mysql/mysql.h>

int main()
{
  // MYSQL* mysql;
  // mysql_init(&mysql);
  MYSQL* mysql = mysql_init(NULL);//直接初始化
  if(mysql == NULL){
    printf("init error\n");
    return -1;
  }
  //链接服务器
  if(mysql_real_connect(mysql,"127.0.0.1","root","123456","db_blog",0,NULL,0) == NULL){
    printf("connect error：%s\n",mysql_error(mysql));
    return -1;
  }
  //设置客户端字符集
  if(mysql_set_character_set(mysql,"utf8") != 0){
    printf("set error\n");
    return -1;
  }
  //选择切换使用的数据库
  if(mysql_select_db(mysql,"db_blog") != 0){
    printf("select error\n");
    return -1;
  }
  //创建表
 //char* sql_str = "create table if not exists tb_stu(id int,name varchar(32),info text,score decimal(4,2),birth datetime);";
  //插入元素
 // char* sql_str = "insert tb_stu values(1,'张','好帅',98.66,'2020-2-20 12:00:00');";
  //修改
 // char* sql_str = "update tb_stu set name ='张文超', info = '更帅' where name = '张';";//把name等于张三的人修改了name和info，
 // char* sql_str = "delete from tb_stu where name = '张文超';";
 //查询（先查询在通过结果集遍历）
  char *sql_str = "select* from tb_stu;";
  int ret = mysql_query(mysql,sql_str);
  if(ret != 0){
    printf("query error:%s\n",mysql_errno(mysql));
    return -1;
  }

  //遍历结果集（遍历）
  MYSQL_RES* res = mysql_store_result(mysql);//存储
  if(res == NULL){
    printf("store error:%s\n",mysql_error(mysql));
    return -1;
  }
  int num_row = mysql_num_rows(res);//行；把res结果集作为参数输入
  int num_col = mysql_num_fields(res);//列
  int i = 0;
  for(i = 0;i<num_row;i++)
  {
    MYSQL_ROW row = mysql_fetch_row(res);//res自带读取位置控制，每次读取都是新的。所以i只是控制别超过行，这里用while也行
    for(int j = 0;j<num_col;j++){
      printf("%s\t",row[j]);
    }
    printf("\n");
  }
  mysql_free_result(res);//关闭结果集，否则资源泄露
  mysql_close(mysql);

  return 0;
}
