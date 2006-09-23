<?php include_once("Base-header.php"); ?>

<h1>Name</h1>

<p>groupdel&nbsp;-- Delete a group</p>

<h1>Usage</h1>

<p class="code"><tt><b>vxgroupdel</b> <b>[</b>-A
<b>[</b><i>command</i><b>]]</b> <b>[</b>-B <b>[</b><i>command</i><b>]]</b>
<b>[</b>-F<b>]</b> <b>[</b>-M <i>backend</i><b>]</b> <i>group</i></tt></p>

<h1>Options</h1>

<table border="1" class="bordered">
  <tr>
    <td class="t1">-A <b>[</b><i>command</i><b>]</b></td>
    <td class="t1">Runs the specified command after the user was successfully
      added. If <tt>-A</tt> is given, but with a zero-length command string,
      the default command in the configuration file is not run.</td>
  </tr>
  <tr>
    <td class="t2">-B <b>[</b><i>command</i><b>]</b></td>
    <td class="t2">Runs the specified command before the user is added. It is
      run always, even if adding fails. If <tt>-B</tt> is given, but with a
      zero-length command string, the default command in the configuration file
      is not run.</td>
  </tr>
  <tr>
    <td class="t1">-F</td>
    <td class="t1"><tt>userdel</tt> will, by default, not delete any groups, of
      which a user is a primary member of. <tt>-F</tt> forces to delete the
      group.</td>
  </tr>
  <tr>
    <td class="t2">-I</td>
    <td class="t2">Interactively prompt for the new group's parameters</td>
  </tr>
  <tr>
    <td class="t1">-M <i>backend</i></td>
    <td class="t1">Uses the specified backend rather than the default one
      listed in the ACCDB configuration file</td>
  </tr>
</table>
</div>

<h1>Description</h1>

<p class="block"><tt>groupdel</tt> deletes the specified group, but will take
care for whether any use has that group as a primary group set. All options
provided are unique to Vitalnix. The configuration file for <tt>groupdel</tt>
is <tt>/etc/vitalnix/groupdel.conf</tt>.</p>

<?php include_once("Base-footer.php"); ?>
