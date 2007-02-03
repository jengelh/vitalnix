<?php include_once("Base-header.php"); ?>

<h1>Description</h1>

<p class="block">The <i>libvxcli</i> library provides a few common functions
for the command-line (CLI) variants of various programs included.</p>

<h1>Structures</h1>

<p class="code"><code>
<b>#</b>include &lt;vitalnix/libvxcli/libvxcli.h&gt;<br />
<br />
<b>struct</b> vxcq_entry {<br />
&nbsp; &nbsp; <b>const char *</b>msg;<br />
&nbsp; &nbsp; <b>const char *</b>prompt;<br />
&nbsp; &nbsp; <b>const char *</b>defl;<br />
&nbsp; &nbsp; <b>int</b> type;<br />
&nbsp; &nbsp; <b>long</b> flags;<br />
&nbsp; &nbsp; <b>void *</b>ptr;<br />
&nbsp; &nbsp; <b>int (*</b>validate<b>)</b>(<b>const struct</b> vxcq_entry <b>*</b>);<br />
};</code></p>

<h2>struct vxcq_entry</h2>

<p class="block">The <code>struct vxcq_entry</code> is used to query a vector
of questions.</p>

<table border="1">
  <tr>
    <td class="t1"><code>-&gt;msg</code></td>
    <td class="t1">tooltip string to be printed, e.g. <code>"Enter a file
      which contains..."</code></td>
  </tr>
  <tr>
    <td class="t2"><code>-&gt;prompt</code></td>
    <td class="t2">a prompt string, e.g. <code>"Filename"</code></td>
  </tr>
  <tr>
    <td class="t1"><code>-&gt;defl</code></td>
    <td class="t1">default answer, if any</td>
  </tr>
  <tr>
    <td class="t2"><code>-&gt;type</code></td>
    <td class="t2">the type of <code>-&gt;ptr</code>. Can be either of
      <code>HXTYPE_STRING</code>, <code>HXTYPE_INT</code>,
      <code>HXTYPE_LONG</code> or <code>HXTYPE_DOUBLE</code></td>
  </tr>
  <tr>
    <td class="t1"><code>-&gt;flags</code></td>
    <td class="t1">a bitmask of <code>CQ_ABORT</code> and/or
      <code>CQ_EMPTY</code> (see below for details). You can specify
      <code>CQ_NONE</code> for convenience in favor of <code>0</code></td>
  </tr>
  <tr>
    <td class="t2"><code>-&gt;ptr</code></td>
    <td class="t2">pointer to where the answer should be stored</td>
  </tr>
  <tr>
    <td class="t1"><code>-&gt;validate</code></td>
    <td class="t1">a function that will be called (if non-<code>NULL</code>) to
      validate the user input. The question is asked again if validation
      returns zero</td>
  </tr>
</table>

<p class="block">All pointers can be <code>NULL</code>.

<h1>Constants</h1>

<p class="block"><code>VXCQ_TABLE_END</code> is a macro that should be used as
a sentinel to a <code>struct vxcq_entry</code>.</p>

<h1>Function overview</h1>

<p class="code"><code>
<b>#</b>include &lt;libvxcli/libvxcli.h&gt;<br />
<br />
<b>char *</b>vxcli_query(<b>const char *</b>msg, <b>const char *</b>prompt, <b>const char *</b>defl, <b>long</b> flags, <b>char *</b>buf, <b>size_t</b> size);<br />
<b>int</b> vxcli_query_v(<b>const struct</b> vxcq_entry <b>*</b>table);
</code></p>

<h2>vxcli_query</h2>

<p class="block">Prints a message, a prompt and the default answer
(<code>msg</code>, <code>prompt</code> and <code>defl</code>, respectively) and
awaits input from stdin. <code>flags</code> is a bitmask that can contain:</p>

<table border="1">
  <tr>
    <td class="t1"><code>VXCQ_ABORT</code></td>
    <td class="t1">If the input consists of a single <code>^A</code> control
      character, the dialog is aborted.</td>
  </tr>
  <tr>
    <td class="t2"><code>VXCQ_EMPTY</code></td>
    <td class="t2">If the input consists of a single <code>^E</code> control
      character, the answer is seen as empty. This is so that a truly empty
      answer (string length = 0) can be used for the default answer.</td>
  </tr>
  <tr>
    <td class="t1"><code>VXCQ_ZNULL</code></td>
    <td class="t1">If the answer is empty, <code>*ptr</code> will be assigned
      <code>NULL</code>.</td>
  </tr>
</table>

<p class="block">If <code>buf</code> is not <code>NULL</code>, the result is
put into buf, writing at most <code>size-1</code> characters plus a
<code>'\0'</code> character. <code>buf</code> is returned. If however,
<code>buf</code> is <code>NULL</code>, the <code>size</code> paramter is
ignored and a dynamic-size <code>hmc_t</code> container is returned. If the
dialog was aborted, <code>NULL</code> is returned.</p>

<h2>vxcli_query_v</h2>

<p class="block">Queries a vector of questions. It takes an array of
<code>struct vxcq_entry</code>s and stores the results of each query in
<code>-&gt;ptr</code>. For strings, this will always create a
<code>hmc_t</code> object. See example below for details. After each question,
the <code>-&gt;validate</code> function is invoked to see if the input is
valid; if not, the query is repeated.</p>

<p class="block">Returns the number of successfully answered questions.</p>

<h1>Example</h1>

<p class="code"><code>
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
</code></p>

<p class="block">Which would show up as:</p>

<p class="code"><code>Enter today's date<br />
(Use &lt;CTRL+A&gt;,&lt;Enter&gt; to abort)<br />
YYYY-MM-DD [2006-03-04] &gt; </code></p>

<?php include_once("Base-footer.php"); ?>
