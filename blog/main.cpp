#include "db.hpp"
#include "httplib.h"

using namespace httplib;
blog_system::TableBlog* table_blog;//使每个函数都能用
blog_system::TableTag* table_tag;

//具体对应的业务处理
void InsertBlog(const Request& req,Response& rsp)
{ //请求数据：jason格式  服务器存储数据：value格式
  //1、从请求中获取正文（因为正文(json格式)，而存入数据库需要value格式，所以要反序列化）
  //2、将json格式字符串进行反序列化
  //3、将反序列化得到的博客信息插入数据库
  Json::Reader reader;
  Json::Value blog;
  bool ret = reader.parse(req.body,blog);//把请求中的正文req.body反序列化转成value，存入blog
  if(ret == false){
    printf("parse error\n");
    rsp.status = 400;
    return ;
  }
  //将博客信息插入数据库
  ret = table_blog->Insert(blog);
  if(ret == false){
    printf("insert errpr\n");
    rsp.status = 400;
    return;
  }
  rsp.status = 200;
  return ;
}
void DeleteBlog(const Request& req,Response& rsp)
{
  //  /blog/123   /blog/(\d+) 对于这个req.matches[0]存的是整个/blog/123 而matches[1]存的是123(字符串形式)
  int blog_id = std::stoi(req.matches[1]);
  bool ret = table_blog->Delete(blog_id);
  if(ret == false){
    printf("delete error\n");
    rsp.status = 400;
    return ;
  }
  rsp.status = 200;
  return ;
}
void UpdateBlog(const Request& req,Response& rsp)
{
  int blog_id = std::stoi(req.matches[1]);//获取要修改的blogid
  Json::Value blog;
  Json::Reader reader;
  bool ret = reader.parse(req.body,blog);//把要修改的正文放入blog中
  if(ret == false){
    printf("Update error\n");
    rsp.status = 400;
    return;
  }
  blog["id"] = blog_id;
  ret = table_blog->Update(blog);
  if(ret == false){
    printf("delete Insert errpr\n");
    rsp.status = 500;
    return;
  }
  rsp.status = 200;
  return ;
}
void GetAllBlog(const Request& req,Response& rsp)
{
  //从数据库获取博客列表数据(先以value格式取出来在进行writer的序列化)
  Json::Value blogs;
  bool ret =table_blog->GetAll(&blogs);//把blog空间的地址传进去接收数据
  if(ret == false){
    printf("GetAll errpr\n");
    rsp.status = 500;
    return;
  }
  Json::FastWriter writer;
  rsp.set_content(writer.write(blogs),"application/json");//content的两个参数，app。。表示是json格式的正文数据
  rsp.status = 200;

  return ;
}
void GetOneBlog(const Request& req,Response& rsp)
{
  int blog_id = std::stoi(req.matches[1]);
  Json::Value blog;
  blog["id"] = blog_id;
  bool ret = table_blog->GetOne(&blog);//传一个参数，是吧blog赋值给blogid
  if(ret == false){
    printf("Get one errpr\n");
    rsp.status = 500;
    return;
  }
  Json::FastWriter writer;
  rsp.set_content(writer.write(blog),"application/json");
  rsp.status = 200;
  return ;
}
void InsertTag(const Request& req,Response& rsp)
{
  Json::Reader reader;
  Json::Value tag;
  bool ret = reader.parse(req.body,tag);//把请求中的正文反序列化转成value，存入tag
  if(ret == false){
    printf("tag parse error\n");
    rsp.status = 400;
    return ;
  }
  //将博客信息插入数据库
  ret = table_tag->Insert(tag);
  if(ret == false){
    printf("insert tag errpr\n");
    rsp.status = 400;
    return;
  }
  rsp.status = 200;
  return ;
}
void DeleteTag(const Request& req,Response& rsp)
{
  int tag_id = std::stoi(req.matches[1]);
  bool ret = table_tag->Delete(tag_id);
  if(ret == false){
    printf("delete tag error\n");
    rsp.status = 400;
    return ;
  }
  rsp.status = 200;
  return ;
}
void UpdateTag(const Request& req,Response& rsp)
{
  int tag_id = std::stoi(req.matches[1]);
  Json::Value tag;
  Json::Reader reader;
  bool ret = reader.parse(req.body,tag);
  if(ret == false){
    printf("Updatetag pares error\n");
    rsp.status = 400;
    return;
  }
  tag["id"] = tag_id;
  ret = table_tag->Update(tag);
  if(ret == false){
    printf("Update error\n");
    rsp.status = 500;
    return;
  }
  rsp.status = 200;
  return ;
}
void GetAllTag(const Request& req,Response& rsp)
{
  //从数据库获取博客列表数据(先以value格式取出来在进行writer的序列化)
  Json::Value tags;
  bool ret =table_tag->GetAll(&tags);//把blog空间的地址传进去接收数据
  if(ret == false){
    printf("GetAll_tag error\n");
    rsp.status = 500;
    return;
  }
  Json::FastWriter writer;
  rsp.set_content(writer.write(tags),"application/json");//app..是格式
  rsp.status = 200;
  return ;
}
void GetOneTag(const Request& req,Response& rsp)
{
  int tag_id = std::stoi(req.matches[1]);
  Json::Value tag;
  tag["id"] = tag_id;
  bool ret = table_tag->GetOne(&tag);
  if(ret == false){
    printf("Get one errpr\n");
    rsp.status = 500;
    return;
  }
  Json::FastWriter writer;
  rsp.set_content(writer.write(tag),"application/json");
  rsp.status = 200;
  return ;
}


int main()
{
  MYSQL* mysql = blog_system::MysqlInit();
  table_blog = new blog_system::TableBlog(mysql);//通过这两个指针来操控数据库中blog、tag的操作。而且他们是全局变量
  table_tag = new blog_system::TableTag(mysql);
  Server server;
  //设置相对根目录的目的：客户端请求静态文件资源时，httplib会根据路径读取文件数据进行响应
  server.set_base_dir("./www");
  //博客信息的增删改查
  server.Post("/blog",InsertBlog);
  //正则表达式：\d-匹配数字字符、+-表示匹配前面的字符多次、()-获取括号内的数据并保存
  //blog/(\d+) 表示匹配以/blog/开头，后面跟了一个数字的字符串格式
  server.Delete(R"(/blog/(\d+))",DeleteBlog);//R"()"该正则表达式表示取消括号内特殊字符的含义
  //标签信息的增删改查
  server.Put(R"(/blog/(\d+))",UpdateBlog);
  server.Get("/blog",GetAllBlog);
  server.Get(R"(/blog/(\d+))",GetOneBlog);

  /**********************标签*******************/
  server.Post("/tag",InsertTag);
  //正则表达式：\d-匹配数字字符、+-表示匹配前面的字符多次、()-获取括号内的数据并保存
  //tag/(\d+) 表示匹配以/tag/开头，后面跟了一个数字的字符串格式
  server.Delete(R"(/tag/(\d+))",DeleteTag);//R"()"该正则表达式表示取消括号内特殊字符的含义
  //标签信息的增删改查
  server.Put(R"(/tag/(\d+))",UpdateTag);
  server.Get("/tag",GetAllTag);
  server.Get(R"(/tag/(\d+))",GetOneTag);

  //监听
  server.listen("0.0.0.0",9000);
  return 0;
}
