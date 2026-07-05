# Note regarding AI usage in this project
This fork, at least currently, is mostly a single-man project. AI tools are used extensivelly
within it for research, codebase search and analysis (since the codebase is inherited from
another project and it will take some time for me to understand the entire project fully), as
well as to writing additional tools, make small code edits and write minor features to speed up
the workflow. That being said, this is not a vibe-coded bullshit maintained by someone without
any programming knowledge that lets agents run loose and commit whatever garbage they produce.
All AI-generated code is heavily scrutinised and human-reviewed before being allowed
in the trunk. We hate low quality slop as most of sane developers do, we don't trust clankers
implicitly. We consider it a tool to enhance developers' work, but not something that should
replace critical thinking and professional insight.

If this is still unacceptable for you, then you might want to look for alternative open source
pixel-art editors. We're not going to waste time on unproductive ideological discussions based
on people's personal feelings towards AI, so don't bother starting such.

## AI co-authored code contribution guidelines
To contribute code to this repo you must first, and foremost, know progrsmming and understand
the code you submit. We expect you to review any AI-generated source code before setting it as
ready for review. Pull requests that show signs of unverified low-quality slop could end up
rejected immediately. While we allow the use of AI, noting the benefits of its usage, at the
same time we expect a degree of respect towards the maintainers who have to review those pull
requests. If the contributor doesn't bother to verify the code vomited by their tool, or, even
worse, doesn't even understand it, we won't bother wasting time doing a review of utter garbage
either.

Any contributions co-authored by AI (meaning changes done directly by an AI agent, or using code
snippets provided by an LLM) should be properly marked by either implicitly saying the changes
were made with the use of AI or by marking the co-authorship in the commit message, e.g.:

    Commit message
    # Some more context to what has been done
    Co-authored-by: Claude <noreply@anthropic.com>

Pull requests that bear signs of AI-generated code that don't admit its authorship might end up
being rejected automatically as well.