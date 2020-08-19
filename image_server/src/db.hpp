#include<cstdio>
#include<iostream>
#include<string>
#include<mutex>
#include<jsoncpp/json/json.h>
#include<mysql/mysql.h>


#define MYSQL_HOSE "0.0.0.0"
#define MYSQL_USER "root"
#define MYSQL_PSWD "902902"
#define MYSQL_NAME "image_server"


namespace image_server
{
    static std::mutex g_mutex;
    //操作句柄初始化(初始化,连接服务器,选择数据库,设置字符集)
    static MYSQL* MysqlInit()
    {
        MYSQL* mysql = NULL;
        mysql = mysql_init(NULL);
        if(mysql == NULL)
        {
            printf("mysql init failed!!\n");
            return NULL;
        }
        if(mysql_real_connect(mysql, MYSQL_HOSE, MYSQL_USER, MYSQL_PSWD, MYSQL_NAME, 0, NULL, 0) == NULL)
        {
            printf("connect mysql server failed : %s\n", mysql_error(mysql));
            return NULL;
        }
        if(mysql_set_character_set(mysql, "utf8") != 0)
        {
            printf("set character failed : %s\n", mysql_error(mysql));
            mysql_close(mysql);
            return NULL;
        }
        return mysql;
    }
    //执行语句
    static bool MysqlQuery(MYSQL* mysql, const std::string sql)
    {
        int ret = mysql_query(mysql, sql.c_str());
        if(ret != 0)
        {
            printf("query sql : [%s] failed : [%s]\n", sql.c_str(), mysql_error(mysql));
            return false;
        }
        return true;
    }
    //释放、关闭操作句柄
    static void MysqlRelease(MYSQL* mysql)
    {
        if(mysql)
        {
            mysql_close(mysql);
        }
    }
    class TableImage
    {
        public:
            TableImage(MYSQL* mysql) : _mysql(mysql){}
        public:
            bool Insert(const Json::Value& image) //插入图片元信息
            {
                //image["id"]["name"]["fsize"]["fpath"]["furl"]["fmd5"]["ultime"]
                #define IMAGE_INSERT "insert into table_image values(null, '%s', %d, '%s', '%s', '%s', now());"
                char sql[4096] = {0};
                sprintf(sql, IMAGE_INSERT, image["name"].asCString(), image["fsize"].asInt(), image["fpath"].asCString(), image["furl"].asCString(), image["fmd5"].asCString());
                if(MysqlQuery(_mysql, sql) == false)
                    return false;
                return true;
            }
            bool Delete(const int image_id) //删除图片元信息
            {
                #define IMAGE_DELETE "delete from table_image where id = %d;"
                char sql[4096] = {0};
                sprintf(sql, IMAGE_DELETE, image_id);
                if(MysqlQuery(_mysql, sql) == false)
                    return false;
                return true;
            }
            bool Update(const int image_id, const Json::Value& image) //修改图片元信息
            {
                #define IMAGE_UPDATE "update table_image set name = '%s' where id = %d;"
                char sql[4096] = {0};
                sprintf(sql, IMAGE_UPDATE, image["name"].asCString(), image_id);
                if(MysqlQuery(_mysql, sql) == false)
                    return false;
                return true;
            }
            bool GetAll(Json::Value* images) //获取所有图片元信息
            {
                #define IMAGE_GETALL "select * from table_image;"
                g_mutex.lock();
                if(MysqlQuery(_mysql, IMAGE_GETALL) == false)
                    return false;
                MYSQL_RES* res = mysql_store_result(_mysql);
                g_mutex.unlock();
                if(res == NULL)
                {
                    printf("store result failed : %s\n", mysql_error(_mysql));
                    return false;    
                }
                uint64_t num_row = mysql_num_rows(res);
                for(int i = 0; i < num_row; i++)
                {
                    MYSQL_ROW row = mysql_fetch_row(res);
                    Json::Value image;
                    image["id"] = std::stoi(row[0]);
                    image["name"] = row[1];
                    image["fsize"] = std::stoi(row[2]);
                    image["fpath"] = row[3];
                    image["furl"] = row[4];
                    image["fmd5"] = row[5];
                    image["ultime"] = row[6];
                    images->append(image); //将每一行添加到images
                }
                mysql_free_result(res);
                return true;
            }
            bool GetOne(const int image_id, Json::Value* image) //获取单个图片元信息
            {
                #define IMAGE_GETONE "select * from table_image where id = %d;"
                char sql[4096] = {0};
                sprintf(sql, IMAGE_GETONE, image_id);
                g_mutex.lock();
                if(MysqlQuery(_mysql, sql) == false)
                    return false;
                MYSQL_RES* res = mysql_store_result(_mysql);
                g_mutex.unlock();
                if(res == NULL)
                {
                    printf("get one image failed : %s\n", mysql_error(_mysql));
                    return false;
                }
                int num_row = mysql_num_rows(res);
                if(num_row != 1)
                {
                    printf("get one image result error\n");
                    mysql_free_result(res);
                    return false;
                }
                MYSQL_ROW row = mysql_fetch_row(res); 
                (*image)["id"] = std::stoi(row[0]);
                (*image)["name"] = row[1];
                (*image)["fsize"] = std::stoi(row[2]);
                (*image)["fpath"] = row[3];
                (*image)["furl"] = row[4];
                (*image)["fmd5"] = row[5];
                (*image)["ultime"] = row[6];
                mysql_free_result(res);
                return true;
            }
        private:
            MYSQL* _mysql;
    };
}
