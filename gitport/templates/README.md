# Historical JOE release

This revision contains a historical release of JOE.  These versions predated
widespread use of source control and were distributed primarily on USENET
and FTP servers.  As such, these do not represent *real* commits, but are
included in the git repository for the sake of completeness.

---
{% for v in versions|reverse %}
## {{ v.name }} {{ v.version }}
{%- if v.date is not none %}
**Released: {{ v.date }}**
{%- else %}
*Release date unknown*
{%- endif %}
{%- if v.source is not none %}

Source recovered from {{ v.source }}
{%- endif -%}
{%- if v.note is not none %}

*Archival notes: {{ v.note }}*
{%- endif -%}
{%- if v.path is none %}

*The source for this release has been lost*
{%- endif -%}
{%- if v.comments is not none %}

Author's comments:

```
{{ v.comments|rtrim }}
```
{%- endif -%}
{%- if v.changelog is not none and (v.announce is none or v.changelog|squash not in v.announce|squash) %}

Changelog:

```
{{ v.changelog|rtrim }}
```
{%- endif -%}
{%- if v.announce is not none %}

Original announcement:
```
{{ v.announce|rtrim }}
```
{%- endif -%}
{%- if not loop.last %}

---
{% endif -%}
{% endfor %}
