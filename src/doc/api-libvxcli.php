<?php include_once("Base-header.php"); ?>

<h1>Description</h1>

<p class="block">The <tt>libvxcli</tt> library provides a few common functions
for the command-line (CLI) variants of various programs included.</p>

<h1>Structures</h1>

<p class="code"><tt>
<b>#</b>include &lt;libvxcli/libvxcli.h&gt;<br />
<br />
<b>struct</b> vxcq_entry {<br />
&nbsp; &nbsp; <b>const char *</b>msg;<br />
&nbsp; &nbsp; <b>const char *</b>prompt;<br />
&nbsp; &nbsp; <b>const char *</b>defl;<br />
&nbsp; &nbsp; <b>int</b> type;<br />
&nbsp; &nbsp; <b>long</b> flags;<br />
&nbsp; &nbsp; <b>void *</b>ptr;<br />
&nbsp; &nbsp; <b>int (*</b>validate<b>)</b>(<b>const struct</b> vxcq_entry <b>*</b>);<br />
};</tt></p>

<h2>struct vxcq_entry</h2>

<p class="block">The <tt>struct vxcq_entry</tt> is used to query a vector of
questions.</p>

<table border="1" class="bordered">
  <tr>
    <td class="t1"><tt>-&gt;msg</tt></td>
    <td class="t1">tooltip string to be printed, e.g. <tt>"Enter a file
      which contains..."</tt></td>
  </tr>
  <tr>
    <td class="t2"><tt>-&gt;prompt</tt></td>
    <td class="t2">a prompt string, e.g. <tt>"Filename"</tt></td>
  </tr>
  <tr>
    <td class="t1"><tt>-&gt;defl</tt></td>
    <td class="t1">default answer, if any</td>
  </tr>
  <tr>
    <td class="t2"><tt>-&gt;type</tt></td>
    <td class="t2">the type of <tt>-&gt;ptr</tt>. Can be either of 
      <tt>HXTYPE_STRING</tt>, <tt>HXTYPE_INT</tt>, <tt>HXTYPE_LONG</tt> or 
      <tt>HXTYPE_DOUBLE</tt></td>
  </tr>
  <tr>
    <td class="t1"><tt>-&gt;flags</tt></td>
    <td class="t1">a bitmask of <tt>CQ_ABORT</tt> and/or 
      <tt>CQ_EMPTY</tt> (see below for details). You can specify
      <tt>CQ_NONE</tt> for convenience in favor of <tt>0</tt></td>
  </tr>
  <tr>
    <td class="t2"><tt>-&gt;ptr</tt></td>
    <td class="t2">pointer to where the answer should be stored</td>
  </tr>
  <tr>
    <td class="t1"><tt>-&gt;validate</tt></td>
    <td class="t1">a function that will be called (if 
      non-<tt>NULL</tt>) to validate the user input. The question is asked
      again if validation returns zero</td>
  </tr>
</table>

<p class="block">All pointers can be <tt>NULL</tt>.

<h1>Constants</h1>

<p class="block"><tt>VXCQ_TABLE_END</tt> is a macro that should be used as a
sentinel to a <tt>struct vxcq_entry</tt>.</p>

<h1>Function overview</h1>

<p class="code"><tt>
<b>#</b>include &lt;libvxcli/libvxcli.h&gt;<br /> 
<br />
<b>char *</b>vxcli_query(<b>const char *</b>msg, <b>const char *</b>prompt, <b>const char *</b>defl, <b>long</b> flags, <b>char *</b>buf, <b>size_t</b> size);<br />
<b>int</b> vxcli_query_v(<b>const struct</b> vxcq_entry <b>*</b>table);
</tt></p>

<h2>vxcli_query</h2>

<p class="block">Prints a message, a prompt and the default answer
(<tt>msg</tt>, <tt>prompt</tt> and <tt>defl</tt>, respectively) and awaits
input from stdin. <tt>flags</tt> is a bitmask that can contain:</p>

<table border="1" class="bordered">
  <tr>
    <td class="t1"><tt>CQ_ABORT</tt></td>
    <td class="t1">If the input consists of a single <tt>^A</tt> control
      character, the dialog is aborted.</td>
  </tr>
  <tr>
    <td class="t2"><tt>CQ_EMPTY</tt></td>
    <td class="t2">If the input consists of a single <tt>^E</tt> control
      character, the answer is seen as empty. This is so that a truly empty
      answer (string length = 0) can be used for the default answer.</td>
  </tr>
</table>

<p class="block">If <tt>buf</tt> is not <tt>NULL</tt>, the result is put into
buf, writing at most <tt>size-1</tt> characters plus a <tt>'\0'</tt> character.
<tt>buf</tt> is returned. If however, <tt>buf</tt> is <tt>NULL</tt>, the
<tt>size</tt> paramter is ignored and a dynamic-size <tt>hmc_t</tt> container
is returned. If the dialog was aborted, <tt>NULL</tt> is returned.</p>

<h2>vxcli_query_v</h2>

<p class="block">Queries a vector of questions. It takes an array of <tt>struct
vxcq_entry</tt>s and stores the results of each query in <tt>-&gt;ptr</tt>. For
strings, this will always create a <tt>hmc_t</tt> object. See example below for
details. After each question, the <tt>-&gt;validate</tt> function is invoked to
see if the input is valid; if not, the query is repeated.</p>

<p class="block">Returns the number of successfully answered questions.</p>

<h1>Example</h1>

<p class="code"><tt>
<b>static int</b> ymd_validate(<b>const char *</b>s) {<br />
&nbsp; &nbsp; return isdigit(s[0]) <b>&amp;&amp;</b> isdigit(s[1]) <b>&amp;&amp;</b> s[2] <b>==</b> '-' <b>&amp;&amp;</b><br />
&nbsp; &nbsp; &nbsp; &nbsp; &nbsp; &nbsp;isdigit(s[3]) <b>&amp;&amp;</b> isdigit(s[4]) <b>&amp;&amp;</b> s[5] <b>==</b> '-' <b>&amp;&amp;</b><br />
&nbsp; &nbsp; &nbsp; &nbsp; &nbsp; &nbsp;isdigit(s[6]) <b>&amp;&amp;</b> isdigit(s[7]) <b>&amp;&amp;</b><br />
&nbsp; &nbsp; &nbsp; &nbsp; &nbsp; &nbsp;isdigit(s[8]) <b>&amp;&amp;</b> isdigit(s[9]);<br />
}<br />
<br />
hmc_t <b>*</b>today_date <b>= NULL</b>;<br />
<b>struct</b> vxcq_entry tbl[] <b>=</b> {<br />
&nbsp; &nbsp; {<br />
&nbsp; &nbsp; &nbsp; &nbsp; .msg &nbsp; &nbsp;&nbsp; <b>=</b> "Enter today's date",<br />
&nbsp; &nbsp; &nbsp; &nbsp; .defl &nbsp; &nbsp;&nbsp;<b>=</b> "2006-03-04",<br />
&nbsp; &nbsp; &nbsp; &nbsp; .prompt &nbsp; <b>=</b> "YYYY-MM-DD",<br />
&nbsp; &nbsp; &nbsp; &nbsp; .type &nbsp; &nbsp; <b>=</b> HXTYPE_STRING,<br />
&nbsp; &nbsp; &nbsp; &nbsp; .ptr&nbsp; &nbsp; &nbsp; <b>= &amp;</b>today_date,<br />
&nbsp; &nbsp; &nbsp; &nbsp; .flags&nbsp; &nbsp; <b>=</b> CQ_ABORT,<br />
&nbsp; &nbsp; &nbsp; &nbsp; .validate <b>=</b> ymd_validate,<br />
&nbsp; &nbsp; },<br />
&nbsp; &nbsp; VXCQ_TABLE_END,<br />
};<br />
vxcli_query_v(tbl);
</tt></p>

<p class="block">Which would show up as:</p>

<p class="code"><tt>Enter today's date<br />
(Use &lt;CTRL+A&gt;,&lt;Enter&gt; to abort)<br />
YYYY-MM-DD [2006-03-04] &gt; </tt></p>

<?php include_once("Base-footer.php"); ?>
