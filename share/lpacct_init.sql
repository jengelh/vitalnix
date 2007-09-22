
drop database vxlpacct;
create database vxlpacct;
use vxlpacct;
create table printlog (
	time        timestamp       not null        default current_timestamp,
	jid         int unsigned,
	queue       varchar(64),
	user        varchar(64),
	title       varchar(255),
	cyan        double          not null        default 0,
	magenta     double          not null        default 0,
	yellow      double          not null        default 0,
	black       double          not null        default 0,
	total       double          not null        default 0,
	pages       int unsigned    not null        default 0,
	confirmed   boolean         not null        default false,
	index       user (user)
) default charset=utf8;

grant select, insert, update, delete
	on vxlpacct.printlog
	to 'lp'@'localhost'
	identified by 'FUNkyaCCounting';
