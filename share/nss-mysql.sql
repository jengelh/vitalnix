# Permission scheme for nss_mysql:

grant select on nss_mysql.users to 'nss'@'%';
grant select on nss_mysql.groups to 'nss'@'%';
grant select on nss_mysql.groupmap to 'nss'@'%';

grant select on nss_mysql.* to 'nss-shadow'@'%' identified by 'foobar';

# Following line is useful for Vitalnix, whose db-mysql.conf will, by default,
# source /etc/nss-mysql*.conf.
#grant select, insert, update, lock tables, execute 
#on nss_mysql.* to 'nss-shadow'@'%';
