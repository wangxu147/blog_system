#include "db.hpp"

int main(){
  MYSQL* mysql = blog_system::MysqlInit();
  blog_system:: TableBlog table_blog(mysql);
  Json::Value blog;
  blog["id"] = 3;
  //blog["title"] = "这是phasdasdp";
  //blog["content"] = "padashp是最好的";
  //table_blog.Insert(blog);
  //table_blog.Delete(1);
  //table_blog.Update(blog);
  table_blog.GetOne(&blog);
  Json::FastWriter writer;
  std::cout<<writer.write(blog)<<std::endl;
  return 0;
}
