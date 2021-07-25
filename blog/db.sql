create database if not exists db_blog;
use db_blog;

create table if not exists tb_tag(
  id int primary key auto_increment comment '标签ID',
  name varchar(32) comment '内容'
);

create table if not exists tb_blog(
  id int primary key auto_increment comment '博客ID',
  tag_id int comment '所属标签',
  title varchar(255) comment '博客标题',
  content text comment '博客内容',
  ctime datetime comment '创建时间'
);


