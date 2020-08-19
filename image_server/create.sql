drop database if exists image_server;
create database if not exists image_server charset = utf8;
 
use image_server;

create table if not exists table_image(
    id int primary key auto_increment,
    name varchar(32) comment "图片名称",
    fsize int comment "图片大小",
    fpath varchar(255) comment "文件存放路径名",
    furl varchar(255) comment "前端访问图片的url",
    fmd5 varchar(64) comment "文件MD5值",
    ultime datetime comment "图片上传时间"
);

insert into table_image values(
    null, "晴天", 1024, "./image/11.jpg", "/image/11.jpg",
    "567yhjkjhgftyuikjhg5678987ygbjkj", now()
);
insert into table_image values(
    null, "雨天", 1025, "./image/22.jpg", "/image/22.jpg",
    "567yhjkjhgftyuikjhg5678987ygbjkj", now()
);
insert into table_image values(
    null, "雪天", 1026, "./image/33.jpg", "/image/33.jpg",
    "567yhjkjhgftyuikjhg5678987ygbjkj", now()
);
