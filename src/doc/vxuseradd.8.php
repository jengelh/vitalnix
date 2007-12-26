<?php include_once("Base-header.php"); ?>

<h1>Name</h1>

<p>useradd&nbsp;-- create a new user</p>

<h1>Syntax</h1>

<p class="code"><code><b>vxuseradd</b> [<b>--after</b> <i>command</i>]
[<b>--before</b> <i>command</i>] [<b>--vxdb</b> <i>database</i>] [<b>-G</b>
<i>group</i>[<b>,</b>...]] [<b>-S</b> <i>level</i>] [<b>-c</b> <i>comment</i>]
[<b>-d</b> <i>home_dir</i>] [<b>-e</b> <i>date</i>] [<b>-f</b> <i>days</i>]
[<b>-g</b> <i>group</i>] [<b>-m</b> [<b>-k</b> <i>skel_dir</i>]] [<b>-s</b>
<i>shell</i>] [<b>-u</b> <i>uid</i> [<b>-o</b>]|<b>-r</b>]
<i>username</i></code></p>

<h1>Description</h1>

<p class="block"><i>vxuseradd</i> will create a new user using the supplied
parameters. The configuration file for <i>vxuseradd</i> is
<code>/etc/vitalnix/useradd.conf</code>.</p>

<h1>Options</h1>

<table border="1">
	<tr>
		<td class="t1n"><code><b>--after</b> <i>command</i></code></td>
		<td class="t1">Runs the specified command after the user was
		successfully added. If <code>--after</code> is given, but with
		a zero-length command string, the default command in the
		configuration file is not run.</td>
	</tr>
	<tr>
		<td class="t2n"><code><b>--before</b>
		<i>command</i></code></td>
		<td class="t2">Runs the specified command before the user is
		added. It is run always, even if adding fails. If
		<code>--before</code> is given, but with a zero-length command
		string, the default command in the configuration file is not
		run.</td>
	</tr>
	<tr>
		<td class="t1n"><code><b>--vxdb</b> <i>database</i></code></td>
		<td class="t1">Uses the specified database rather than the
		default one listed in the VXDB configuration file.</td>
	</tr>
	<tr>
		<td class="t2n"><code><b>-G</b>
		<i>group</i>[<b>,</b>...]</code></td>
		<td class="t2">The supplemental (secondary) groups the user
		should be a member in, separated by comma. <i>group</i> can
		either be the group's name or its GID. (UNSUPPORTED)</td>
	</tr>
	<tr>
		<td class="t2n"><code>-S</code></td>
		<td class="t2">Uses the "split home" feature. This will create the home
			directory as <code>/home/u/username</code> rather than the default
			<code>/home/username</code>. Specifying <code>-S</code> twice (which is
			the maximum) will result in a two-level split, i.e. 
			<code>/home/u/us/username</code>.	The <code>-d</code> option overrides
			this one.</td>
	</tr>
	<tr>
		<td class="t1n"><code>-c <i>comment</i></code></td>
		<td class="t1">The comment field for the new user. It can be in turn again
			split up in fields, separated by comma. Usually, the real name of the
			user is stored in the first field, the others are left for other
			commentary or data.</td>
	</tr>
	<tr>
		<td class="t2n"><code>-d <i>dir</i></code></td>
		<td class="t2">The new user will get <code>dir</code> as his home
			directory.	This option alone does not create his home directory, but
			rather only writes that directory information into the user database. You
			usually want to pass <code>-m</code> also along to create the directory.
			This option overrides <code>-S</code>.</td>
	</tr>
	<tr>
		<td class="t1n"><code>-e <i>date</i></code></td>
		<td class="t1">Date on which the account expires. Date formats must be
			YYYY-MM-DD. The default is to not let the account expire.</td>
	</tr>
	<tr>
		<td class="t2n"><code>-f <i>days</i></code></td>
		<td class="t2">The number of days which need to pass after a password
			expiration (that is not <code>-e</code>!) until the account is locked
			down.</td>
	</tr>
	<tr>
		<td class="t1n"><code>-g <i>group</i></code></td>
		<td class="t1">The group name or number of the user's initial (primary)
			group. <code>group</code> can either be the group's name or its GID.</td>
	</tr>
	<tr>
		<td class="t2n"><code>-k <i>dir</i></code></td>
		<td class="t2">The skeleton directory to use for the new user. Contents
			from this directory will be copied into the user's.</td>
	</tr>
	<tr>
		<td class="t1n"><code>-m</code></td>
		<td class="t1">Create the user's home directory. The default is to create a
			directory called the same name as the user in <code>/home</code>, or what
			you specified as <code>HOME</code> in the configuration file, or if you
			passed <code>-S</code> or <code>-d</code>. If this option is given, an
			implicit <code>-k /etc/skel</code> (or <code>SKEL</code>) takes effect,
			unless an explicit <code>-k</code> is specified.</td>
	</tr>
	<tr>
		<td class="t2n"><code>-o</code></td>
		<td class="t2">If a user with the explicit UID given by <code>-u</code>
			already exists, you can override the error message with this option, to
			create non-unique UIDs.</td>
	</tr>
	<tr>
		<td class="t1n"><code>-p <i>pass</i></code></td>
		<td class="t1">The password to use for the new user. It must already be
			encrypted when passed with <code>-p</code>.</td>
	</tr>
	<tr>
		<td class="t2n"><code>-r</code></td>
		<td class="t2">Create a system user, that is, with a lower UID than the
			hardcoded default (100), or what is specified for <code>MIN_UID</code> in
			the autouid configuration file. The <code>-u</code> option overrides this
			one.</td>
	</tr>
	<tr>
		<td class="t1n"><code>-s <i>shell</i></code></td>
		<td class="t1">Use <code>shell</code> as the default command interpreter
			for the new user. The default is <code>/bin/bash</code> or whatever you
			specified as <code>SHELL</code> in the configuration file.</td>
	</tr>
	<tr>
		<td class="t2n"><code>-u <i>uid</i></code></td>
		<td class="t2">The numerical user id of the new user. The default is to
			assign the user the next greatest UID. This option overrides
			<code>-r</code>.</td>
	</tr>
</table>

<?php include_once("Base-footer.php"); ?>
