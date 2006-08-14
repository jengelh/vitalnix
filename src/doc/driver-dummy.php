<?php include_once("Base-header.php"); ?>

<h1>Description</h1>

<p class="block">The "dummy" driver does exactly what it name suggests. For
read operations, it returns success, and for write operations, it returns
failure. The exact table is:</p>

<table border="1" class="bordered">
  <tr>
    <td class="t1">-&gt;init()<br />
      -&gt;open()</td>
    <td class="t1"><tt>1</tt> (success)</td>
  </tr>
  <tr>
    <td class="t2">-&gt;close()<br />
      -&gt;deinit()</td>
    <td class="t2">void</td>
  </tr>
  <tr>
    <td class="t1">-&gt;modctl(<tt>command=PDB_COUNT_USERS</tt>)<br />
      -&gt;modctl(<tt>command=PDB_COUNT_GROUPS</tt>)</td>
    <td class="t1"><tt>0</tt> (no users/groups)</td>
  </tr>
  <tr>
    <td class="t2">-&gt;modctl(<tt>command=</tt>*)</td>
    <td class="t2"><tt>-ENOSYS</tt> ("Function not implemented")</td>
  </tr>
  <tr>
    <td class="t1">-&gt;lock()<br />
      -&gt;unlock()</td>
    <td class="t1"><tt>1</tt> (success)</td>
  </tr>
  <tr>
    <td class="t2">-&gt;useradd()<br />
      -&gt;usermod()<br />
      -&gt;userdel()<br />
      -&gt;groupadd()<br />
      -&gt;groupmod()<br />
      -&gt;groupdel()</td>
    <td class="t2"><tt>-EPERM</tt> ("Operation not permitted")</td>
  </tr>
  <tr>
    <td class="t1">-&gt;usertrav_init()<br />
      -&gt;grouptrav_init()</td>
    <td class="t1">non-<tt>NULL</tt> (traverser init succeeded)</td>
  </tr>
  <tr>
    <td class="t2">-&gt;usertrav_walk()<br />
      -&gt;grouptrav_walk()</td>
    <td class="t2"><tt>0</tt> (no more users/groups)</td>
  </tr>
  <tr>
    <td class="t1">-&gt;usertrav_free()<br />
      -&gt;grouptrav_free()</td>
    <td class="t1">void</td>
  </tr>
</table>

<p class="block">To be precise, the dummy code is contained within
<i>libvxpdb</i>, the <i>drv_dummy</i> file just allows direct access to it.
Every module that does not provide its own functions will get a respective
dummy substitute in its vtable upon loading, as is for example the case with
<i>drv_mysql</i> which does not provide its own <i>-&gt;lock</i>.</p>

<?php include_once("Base-footer.php"); ?>
