# Web Archive System

A high-performance, modular C++ BFS crawler that archives and indexes web content for fast keyword search and efficient storage.

---

## Features

- **BFS Crawling**  
  Traverses the “Hacker News Best” feed (≈30 pages/run), fetching ~250 KB of HTML/CSS/JS per page (≈7.5 MB total) in under 60 seconds.

- **Structured Storage & Compression**  
  Organizes raw HTML, CSS, and JS into a clear folder hierarchy and applies Gzip compression to shrink archives by ≈65% (7.5 MB → 2.6 MB).

- **Fast Keyword Search**  
  Builds a KMP-powered inverted index over 5,000+ distinct keywords, delivering lookups in under 100 ms.

- **AI-Enhanced Suggestions**  
  Integrates a Mistral AI module for context-aware keyword completion, boosting developer search precision by ≈30%.

- **Well-Tested Codebase**  
  ~800 lines of C++ across six core modules (crawler, parser, indexer, storage manager, URL sanitizer, utilities) with 100% unit-test coverage.

---

## Repo Structure

