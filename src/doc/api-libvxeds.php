<?php include_once("Base-header.php"); ?>

<h1>Description</h1>

<p class="block"><i>libvxeds</i> is the library for External Data Sources
(EDS). It converts data sources of various input types to the internal
<code>vxeds_entry</code> structure used by <i>libvxmdsync</i>.</p>

<h1>Structures</h1>

<p class="code"><code>
<b>#</b>include &lt;vitalnix/libvxeds/libvxeds.h&gt;<br />
<br />
<b>struct</b> vxeds_entry {<br />
&nbsp; &nbsp; <b>char *</b>username;<br />
&nbsp; &nbsp; <b>char *</b>first_name;<br />
&nbsp; &nbsp; <b>char *</b>surname;<br />
&nbsp; &nbsp; <b>char *</b>full_name; // concatenation of first_name + surname<br />
&nbsp; &nbsp; <b>char *</b>pvgrp; &nbsp; &nbsp; // private group descriptor<br />
&nbsp; &nbsp; <b>char *</b>uuid; &nbsp; &nbsp; &nbsp;// external unique user identifier<br />
};</code></p>

<h1>Function overview</h1>

<p class="code"><code>
<b>#</b>include &lt;libvxeds/libvxeds.h&gt;<br />
<br />
<b>int</b> vxeds_open(<b>const char *</b>id, <b>const char *</b>fmt, <b>void **</b>state);<br />
<b>int</b> vxeds_read(<b>void *</b>state, <b>struct</b> vxeds_entry <b>*</b>entry);<br />
<b>void</b> vxeds_close(<b>void *</b>state);<br />
<b>const char *</b>vxeds_derivefromname(<b>const char *</b>filename);<br />
</code></p>

<h2>vxeds_open</h2>

<p class="block">Open the data source identified by <code>id</code>. This is
mostly a filename, but the exact interpretation depends on <code>fmt</code>.
<code>fmt</code> must denote the type of the data source. <code>state</code>
must be a pointer to a valid local variable (see example below) which is then
later to be passed on subsequent <code>vxeds_read()</code> calls. Returns an
AEE code; the following extra errors can happen:</p>

<table border="1">
  <tr>
    <td class="t1"><code>-EINVAL</code></td>
    <td class="t1">No handler registered for the format given by
      <code>fmt</code></td>
  </tr>
  <tr>
    <td class="t2"><code>-ENOMEM</code></td>
    <td class="t2">Out of memory</td>
  </tr>
</table>

<h2>vxeds_read</h2>

<p class="block">Reads the next entry from the data source and returns it in
normalized form in <code>entry</code>. The members of the <code>struct
vxeds_entry</code> are allocated (if a string), so they must be freed
afterwards, preferably using the simple <code>vxeds_free_entry()</code>. (Note:
The username is constructed within <i>libvxmdsync</i>.)</p>

<p class="block">Returns zero on EOF, positive non-zero on success, or negative
non-zero for error. Most common is <code>-EINVAL</code> if the parser has
detected an incosistency in the data source. A parsing module is free to try to
continue to read the data source after an inconsistency, or return zero on the
call following the one that returned <code>-EINVAL</code>. In other words, the
return codes of repeated calls to <code>vxeds_read()</code> can look as
follows:</p>

<ul>
  <li>with recovery&nbsp;-- <code>1 1 1 -EINVAL 1 1 1 0</code></li>
  <li>stop after incosistency&nbsp;-- <code>1 1 1 -EINVAL 0</code></li>
</ul>

<h2>vxeds_close</h2>

<p class="block">Close the data source associated with <code>state</code>.</p>

<h2>vxeds_derivefromname</h2>

<p class="block">Tries to figure out the filetype from the filename and returns
a pointer to a string which could be passed as <code>fmt</code> to
<code>vxeds_open()</code>.</p>

<h2>vxeds_free_entry</h2>

<p class="block">Frees all members of a <code>struct vxeds_entry</code>.</p>

<h1>Examples</h1>

<p class="code"><code>
<b>struct</b> vxeds_entry e;<br />
<b>void *</b>eds_state;<br />
<b>int</b> ret;<br />
<br />
if((ret <b>=</b> vxeds_open("20060101.xml", "xml", <b>&amp;</b>eds_state)) <b>&lt;=</b> 0)<br />
&nbsp; &nbsp; abort();<br />
while((ret <b>=</b> vxeds_read(eds_state, <b>&amp;</b>e) <b>&gt;</b> 0) {<br />
&nbsp; &nbsp; do_something(<b>&amp;</b>e);<br />
&nbsp; &nbsp; vxeds_free_entry(<b>&amp;</b>e);<br />
}<br />
vxeds_close(eds_state);
</code></p>

<?php include_once("Base-footer.php"); ?>
