
drop database lpacct;
create database lpacct;
use lpacct;
create table printlog (
    time        timestamp       not null        default current_timestamp,
    jid         int(11) unsigned,
    queue       varchar(64),
    user        varchar(64),
    cyan        double(9, 5)          not null        default 0,
    magenta     double(9, 5)          not null        default 0,
    yellow      double(9, 5)          not null        default 0,
    black       double(9, 5)          not null        default 0,
    total       double(9, 5)          not null        default 0,
    index       user (user)
) default charset=utf8;

grant insert, select
    on lpacct.printlog
    to 'lp'@'localhost'
    identified by 'FUNkyaCCounting';
grant lock tables
    on lpacct.*
    to 'lp'@'localhost'
    identified by 'FUNkyaCCounting';
