from docx import Document
from pathlib import Path
p=Path(r"c:\Saadat\Class\Spring 2026\CSE461\Project\CSE461 Project Proposal Template.docx")
d=Document(str(p))
for i,para in enumerate(d.paragraphs):
    t=para.text.replace('\n','\\n')
    print(f"{i:03d}: {t}")
