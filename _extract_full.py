from docx import Document
from docx.oxml.ns import qn
import sys, os

def extract_full(path):
    doc = Document(path)
    
    print("=" * 60)
    print("FULL DOCUMENT EXTRACTION")
    print("=" * 60)
    
    # Extract all body elements in order (paragraphs + tables)
    body = doc.element.body
    for elem in body:
        if elem.tag == qn('w:p'):
            # Paragraph
            texts = []
            for run in elem.findall('.//'+qn('w:t')):
                if run.text:
                    texts.append(run.text)
            text = ''.join(texts).strip()
            # Get style
            pPr = elem.find(qn('w:pPr'))
            style = ""
            numId = ""
            if pPr is not None:
                pStyle = pPr.find(qn('w:pStyle'))
                if pStyle is not None:
                    style = pStyle.get(qn('w:val'), '')
                numPr = pPr.find(qn('w:numPr'))
                if numPr is not None:
                    ilvl = numPr.find(qn('w:ilvl'))
                    nid = numPr.find(qn('w:numId'))
                    if ilvl is not None and nid is not None:
                        numId = f"[list lvl={ilvl.get(qn('w:val'))} numId={nid.get(qn('w:val'))}]"
            
            if text:
                prefix = ""
                if 'Heading' in style:
                    level = style.replace('Heading', '').strip()
                    prefix = f"{'#' * int(level)} " if level.isdigit() else "## "
                if numId:
                    prefix = f"  {numId} "
                print(f"{prefix}{text}")
            elif not text and style:
                print(f"[empty para, style={style}]")
                
        elif elem.tag == qn('w:tbl'):
            # Table
            print("\n--- TABLE START ---")
            rows = elem.findall('.//'+qn('w:tr'))
            for r_idx, row in enumerate(rows):
                cells = row.findall(qn('w:tc'))
                cell_texts = []
                for cell in cells:
                    paras = cell.findall(qn('w:p'))
                    ctxt = []
                    for p in paras:
                        runs = p.findall('.//'+qn('w:t'))
                        ctxt.append(''.join(r.text or '' for r in runs))
                    cell_texts.append(' | '.join(ct.strip() for ct in ctxt if ct.strip()))
                print(f"  Row {r_idx}: {' || '.join(cell_texts)}")
            print("--- TABLE END ---\n")
    
    # Also check for any sections/headers/footers
    print("\n" + "=" * 60)
    print("DOCUMENT PROPERTIES")
    print("=" * 60)
    print(f"Sections: {len(doc.sections)}")
    print(f"Paragraphs: {len(doc.paragraphs)}")
    print(f"Tables: {len(doc.tables)}")
    
    # List all unique styles used
    styles_used = set()
    for p in doc.paragraphs:
        if p.style:
            styles_used.add(p.style.name)
    print(f"Styles used: {sorted(styles_used)}")

if __name__ == "__main__":
    sys.stdout.reconfigure(encoding='utf-8', errors='replace')
    extract_full(sys.argv[1])
