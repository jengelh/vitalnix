# Usual permission scheme for nss_mysql

# Note that the hostname wildcard '%' does NOT include localhost.
grant select on nss_mysql.users to 'nss'@'localhost';
grant select on nss_mysql.groups to 'nss'@'localhost';
grant select on nss_mysql.groupmap to 'nss'@'localhost';

grant select on nss_mysql.* to 'nss-shadow'@'localhost' identified by 'foobar';

# Following line is useful for Vitalnix, whose db-mysql.conf will, by default,
# source /etc/nss-mysql*.conf.
#grant select, insert, update, lock tables, execute
#on nss_mysql.* to 'nss-shadow'@'localhost';
#
# However, you should configure Vitalnix so that it uses a separate account:
#grant select, insert, update, lock tables, execute
#on nss_mysql.* to 'nss-vitalnix'@'localhost';
