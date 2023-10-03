import os
import glob

DOCS_DIR = "external/feather-docs/content/_guides"
OUT_DIR = "src/assets/docs"

CATEGORY_MAP = {
    "getting-started": "1. Getting started",
    "howto": "2. How to",
    "faq": "3. Faq",
    "advanced": "4. Advanced",
    "troubleshooting": "5. Troubleshooting",
    "help": "6. Help",
}

if not os.path.isdir(DOCS_DIR):
    print("feather-docs submodule not found. Run `git submodule update --init --recursive`")
    exit(1)

outfiles = glob.glob(f"{OUT_DIR}/*.md")
for file in outfiles:
    os.remove(file)

files = glob.glob(f"{DOCS_DIR}/*.md")

for file in files:
    with open(file) as f:
        doc = f.read()

    if not doc:
        continue

    # yaml frontmatter missing
    if doc.count("---") < 2:
        continue

    _, front, body = doc.split("---", 2)
    front = {x: y.strip(" \"") for (x, y) in [x.split(':', 1) for x in front.splitlines()[1:]]}

    if not all((x in front) for x in ['category', 'nav_title']):
        continue

    if front['category'] not in CATEGORY_MAP:
        continue

    title = front['nav_title'].replace("(", "\\(").replace(")", "\\)")

    # We use this format to insert metadata while preventing it from showing up in QTextBrowser
    # This is easier than adding item tags to .qrc files and parsing the XML
    # We need to be able to setSource on a resource directly, otherwise history doesn't work
    docString = f"""
[nav_title]: # ({title})
[category]: # ({CATEGORY_MAP[front['category']]})
    
## {front['title']}
{body}
    """

    _, filename = file.rsplit('/', 1)

    with open(f"{OUT_DIR}/{filename}", 'w') as f:
        print(filename)
        f.write(docString)
