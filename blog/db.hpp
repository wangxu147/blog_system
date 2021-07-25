#include <iostream>
#include <jsoncpp/json/json.h>
#include <mysql/mysql.h>
#include <mutex>
#include <mysql/mysql.h>

#define  MYSQL_HOST "127.0.0.1"
#define  MYSQL_USER "root"
#define  MYSQL_PSWD "123456"
#define MYSQL_DB "db_blog"

namespace blog_system{
  static std::mutex g_mutex;
  MYSQL* MysqlInit()//数据库的初始化操作，包括连接数据库等。创建好的数据库会返回一个数据库操作句柄用于数据库的操作
  {//搭建mysql的客户端
    MYSQL* mysql;
    //初始化
    mysql = mysql_init(NULL);
    if(mysql == NULL){
      printf("init mysql erro \n");
      return NULL;
    }
    //链接服务器
    if(mysql_real_connect(mysql,MYSQL_HOST,MYSQL_USER,MYSQL_PSWD,NULL,0,NULL,0) == NULL){//都是建议
      printf("connect error:%s\n",mysql_errno(mysql));
      mysql_close(mysql);//链接服务器失败就要把创建的服务器删除掉
      return NULL;
    }
    //设置字符集
    if(mysql_set_character_set(mysql,"utf8")!= 0){
      printf("set error:%s\n",mysql_errno(mysql));
      mysql_close(mysql);//链接服务器失败就要把创建的服务器删除掉
      return NULL;
    }
    //选择数据库
    if(mysql_select_db(mysql,MYSQL_DB) != 0){ 
      printf("select error:%s\n",mysql_errno(mysql));
      mysql_close(mysql);//链接服务器失败就要把创建的服务器删除掉
      return NULL;
    }
    return mysql;

  }
  void MysqlRelease(MYSQL* mysql)//数据库的销毁
  {
    if(mysql){
      mysql_close(mysql);
    }
  }
  bool MysqlQuery(MYSQL* mysql,const char* sql)//只是防止多次打印.这个const是因为getone中传过来的全是字符串具有常属性
  {
    int ret = mysql_query(mysql,sql);//只是把要做什么操作统一放入sql中，然后统一在MysqlQuery中
    if(ret != 0){
        printf("query error\n");
        return false;
    }
    return true;
  }

  class TableBlog{
    public:
      TableBlog(MYSQL*mysql):_mysql(mysql){}
      bool Insert(Json::Value & blog){//从blog取出信息组织sql，将数据插入数据库；数据库插入的数据是反序列化的value类型
#define INSERT_BLOG "insert tb_blog values(null,'%d','%s','%s',now());"
        int len = blog["content"].asString().size()+4096;//asString,取内容的字符串长度
        char* tmp = (char*)malloc(len);//char tem[4096] = {0};直接开tmp的大小也行
        sprintf(tmp,INSERT_BLOG,blog["tag_id"].asInt(),
            blog["title"].asCString(),
            blog["content"].asCString()
            );//这只是把宏定义的那句话放到tmp中，sprintf把其中需要的数据放进去组织成一个字符串放tmp，最后传个query函数中的sql
        bool ret = MysqlQuery(_mysql,tmp);
        free(tmp);
        return ret;
      }

      bool Delete(int blog_id){//根据id删除具体哪个博客
#define DELETE_BLOG "delete from tb_blog where id = %d;"
        char tmp[1024] = {0};
        sprintf(tmp,DELETE_BLOG,blog_id);//上面blog["tag_id"]是因为传进来的是blog，要取其中的内容。而删除已经传进来id了直接用
        bool ret = MysqlQuery(_mysql,tmp);
        return ret;

      }

      bool Update(Json::Value& blog){//这里用&是因为需要传过来一个Value的值进行修改
#define UPDATE_BLOG "update tb_blog set tag_id = %d,title = '%s',content = '%s' where id = %d;"
        int len = blog["content"].asString().size()+4096;
        char* tmp = (char*)malloc(len);
        sprintf(tmp,UPDATE_BLOG,blog["tag_id"].asInt(),
            blog["title"].asCString(),
            blog["content"].asCString(),
            blog["id"].asInt()
            );
         bool ret = MysqlQuery(_mysql,tmp) ;
         free(tmp);
         return ret;
      }

      bool GetAll(Json::Value * blogs){//这是获取blog的数据(通常是列表展示)
        //对于获取，需要先遍历、保存结果集、遍历结果集
        //遍历
#define GETALL_BLOG "select id,tag_id,title,ctime from tb_blog;"//不需要获取正文只要其他关键元素就行。select* from tbname是获取全部。
        g_mutex.lock();//执行语句和存储结果集不是原子操作，可能A执行的时候切换到B了，保存的B的结果集
        bool ret = MysqlQuery(_mysql,GETALL_BLOG);//因为GETALL语句中没有要素要整合，所以直接传
        if(ret == false){
          g_mutex.unlock();//失败的时候返回也要解锁
          return false;
        }
        //保存结果集
        MYSQL_RES* res = mysql_store_result(_mysql);
        g_mutex.unlock();//成功结束的时候解锁
        if(res == NULL){
          printf("store error\n");
          return false;
        }
        //遍历结果集
        int row_num = mysql_num_rows(res);
        //因为已经知道了列有多少个（也就是每个对象中有什么元素，直接输出就好
        for(int i=0; i<row_num; i++){
          MYSQL_ROW row = mysql_fetch_row(res);
          Json::Value blog;
          blog["id"] = std::stoi(row[0]);
          blog["tag_id"] = std::stoi(row[1]);
          blog["title"] = row[2];
          blog["ctime"] = row[3];
          blogs->append(blog);//追加；因为一个blog是一个对象，而blogs是一个Json组，里面有很多对象
        }
        mysql_free_result(res);//结束后释放结果集
        return true;
      }

      bool GetOne(Json::Value* blog){//返回单个博客，包含正文信息
#define GETONE_BLOG " select tag_id,title,content,ctime from tb_blog where id = %d;"
        char tmp[1024] = {0}; 
        sprintf(tmp,GETONE_BLOG,(*blog)["id"].asInt());//这里blog传进来是地址，而且[]优先级高，要先解引用
        g_mutex.lock();
        bool ret = MysqlQuery(_mysql,tmp);
        if(ret == false){
          printf("query.. error\n");
          g_mutex.unlock();
          return false;
        }
        MYSQL_RES* res = mysql_store_result(_mysql);
        g_mutex.unlock();
        if(res == NULL){
          printf("tore error\n");
          return false;
        }

        int row_num = mysql_num_rows(res);
        if(row_num != 1){
          printf("GET NOE ERROR\n");
          mysql_free_result(res);
          return false;
        }

        MYSQL_ROW row = mysql_fetch_row(res);
        (*blog)["tag_id"] = std::stoi(row[0]);
        (*blog)["title"] = row[1];
        (*blog)["content"] = row[2];
        (*blog)["ctime"] = row[3];
        mysql_free_result(res);
        return true;
      }
    private:

      MYSQL* _mysql;
  };


  
//博客标签
  class TableTag{
    public:
      TableTag(MYSQL* mysql):_mysql(mysql){}

      bool Insert(Json::Value &tag){//要插入什么内容，引用传参
#define INSERT_TAG "insert tb_tag values(null,'%s');"//tag只有序号（且自增）内容两个
        char tmp[1024] = {0};
        sprintf(tmp,INSERT_TAG,tag["name"].asCString());
        return MysqlQuery(_mysql,tmp);
      }

      bool Delete(int tag_id){
#define DELETE_TAG "delete from tb_tag where id = %d;"
        char tmp[1024] = {0};
        sprintf(tmp,DELETE_TAG,tag_id);
        return MysqlQuery(_mysql,tmp);
      }

      bool Update(Json::Value &tag){//引用传参
#define UPDATE_TAG "update tb_tag set name = '%s' where id = %d;"
        char tmp[1024] = {0};
        sprintf(tmp,UPDATE_TAG,tag["name"].asCString(),tag["id"].asInt());
        return MysqlQuery(_mysql,tmp);
      }

      bool GetAll(Json::Value *tags){//把获取的结果传入到tags中
#define GETALL_TAG "select id,name from tb_tag;"
        g_mutex.lock();
        bool ret = MysqlQuery(_mysql,GETALL_TAG);
        if(ret == false){
          g_mutex.unlock();
          return false;
        }
        //保存结果集
        MYSQL_RES* res = mysql_store_result(_mysql);
        g_mutex.unlock();
        if(res == NULL){
          printf("store error\n");
        }
      //遍历结果集
      int row_num = mysql_num_rows(res);
      for(int i = 0;i<row_num;i++){
        MYSQL_ROW row = mysql_fetch_row(res);
        Json::Value tag;
        tag["id"] = std::stoi(row[0]);
        tag["name"] = row[1];
        tags->append(tag);//往结果集tags中加每个对象tag
      }
      mysql_free_result(res);
      return true;

      }

      bool GetOne(Json::Value *tag){
#define GETONE_TAG "select name from tb_tag where id = %d;"
       char tmp[1024] = {0};
       sprintf(tmp,GETONE_TAG,(*tag)["id"].asInt());
        g_mutex.lock();
       bool ret = MysqlQuery(_mysql,tmp);
       if(ret == false){
         g_mutex.unlock();
         return false;
       }

       MYSQL_RES* res = mysql_store_result(_mysql);
       g_mutex.unlock();
       if(res ==NULL){
         printf("store error\n");
         return false;
       }
       int row_num = mysql_num_rows(res);
       if(row_num != 1){
         printf("get one error\n");
         mysql_free_result(res);
         return false;
       }

       MYSQL_ROW row = mysql_fetch_row(res);
      (*tag)["name"] = row[0];
       mysql_free_result(res);
       return true;
      }

    private:
      MYSQL* _mysql;
  };


}
