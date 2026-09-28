# Note regarding AI usage in this project
This fork is, at least for now, mostly a one-person project. AI tools are used extensively in it: for
research, for searching and analysing the codebase (the code is inherited from another project, and it
takes a single person a long time to understand all of it), and for writing additional tools,
translations and smaller features to speed up the workflow.

That doesn't mean AI output goes into the project unchecked. Every piece of AI-generated code is
carefully reviewed by a human before it's allowed into `trunk`, and low-quality output is rejected just
like any other low-quality contribution. We see AI as a tool for repetitive and routine work, so that
the developer can focus on the interesting problems and features. It doesn't replace critical thinking
or professional judgement. On this topic the maintainer shares a view
[similar to Linus Torvalds'](https://lore.kernel.org/linux-media/CAHk-=wi4zC+Ze8e+p3tMv8TtG_80KzsZ1syL9anBtmEh5Z40vg@mail.gmail.com/).

Besprited itself has **no generative AI features**: the editor doesn't generate or alter artwork with
AI. The notes above are only about how the application's code is developed.

If this approach doesn't work for you, that's understandable. There are other open source pixel-art
editors to choose from, and code contributions from people who prefer not to use AI tools are just as
welcome, since they reduce how much the project relies on them.

We're happy to discuss concrete problems with specific code, whoever or whatever wrote it. We won't,
however, take part in general debates about whether AI tools should be used at all; such discussions
in issues and pull requests will be closed.

## AI co-authored code contribution guidelines
To contribute code to this repository you need to know how to program and understand the code you
submit. Review any AI-generated code yourself before marking a pull request as ready for review.
Pull requests that look like unverified, low-quality AI output may be closed without a full review:
reviewing takes the maintainers' time, and we expect contributors to have done their part first.

Contributions co-authored by AI (changes made directly by an AI agent, or code snippets provided by an
LLM) must be marked as such, either by saying so in the pull request or by adding a co-authorship line
to the commit message, e.g.:

    Commit message
    # Some more context to what has been done
    Co-authored-by: Claude <noreply@anthropic.com>

Pull requests that show clear signs of AI-generated code without disclosing it may also be rejected.
