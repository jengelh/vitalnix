<?php include_once("Base-header.php"); ?>

<h1>Name</h1>

<p>groupmod&nbsp;-- Modify a group</p>

<h1>Synopsis</h1>

<p class="code"><tt><b>vxgroupmod</b> <b>[</b>-A
<b>[</b><i>command</i><b>]]</b> <b>[</b>-B <b>[</b><i>command</i><b>]]</b>
<b>[</b>-I<b>]</b> <b>[</b>-M <i>backend</i><b>]</b> <b>[</b>-g <i>gid</i>
<b>[</b>-o<b>]]</b> <b>[</b>-n <i>group_name<b>]</b> <i>group</i></tt></p>

<h1>Options</h1>

<table border="1" class="bordered">
  <tr>
    <td class="t1">-A [command]</td>
    <td class="t1">Runs the specified command after the user was successfully
      added. If <tt>-A</tt> is given, but with a zero-length command string,
      the default command in the configuration file is not run.</td>
  </tr>
  <tr>
    <td class="t2">-B [command]</td>
    <td class="t2">Runs the specified command before the user is added. It is
      run always, even if adding fails. If <tt>-B</tt> is given, but with a
      zero-length command string, the default command in the configuration file
      is not run.</td>
  </tr>
  <tr>
    <td class="t1">-I</td>
    <td class="t1">Interactively prompt for the new group's parameters</td>
  </tr>
  <tr>
    <td class="t2">-M backend</td>
    <td class="t2">Uses the specified backend rather than the default one
      listed in the ACCDB configuration file</td>
  </tr>
  <tr>
    <td class="t1">-g gid</td>
    <td class="t1">Changes the user's identification number. It must be unique,
      unless <tt>-o</tt> is given.</td>
  </tr>
  <tr>
    <td class="t2">-o</td>
    <td class="t2">If a group with the explicit GID given by <tt>-g</tt>
      already exists, you can override the error message with this option, to
      create non-unique GIDs.</td>
  </tr>
  <tr>
    <td class="t1">-n name</td>
    <td class="t1">Changes the group's name, which must be unique.</td>
  </tr>
</table>
</div>

<h1>Description</h1>

<p class="block"><tt>groupmod</tt> updates the group's info with the data given
on the command line. Not specified options will not change the corresponding
fields in the database.</p>

<p class="block">If <tt>-I</tt> is passed, the user is interactively asked for
what parameters shall be changed, by presenting a default value, which he may
either accept (by leaving the answer empty) or specifying a new value. The
<tt>-A</tt>, <tt>-B</tt>, <tt>-I</tt>, and <tt>-M</tt> options are unique to
Vitalnix. The configuration file for <tt>groupmod</tt> is
<tt>/etc/vitalnix/groupmod.conf</tt>.</p>

<?php include_once("Base-footer.php"); ?>