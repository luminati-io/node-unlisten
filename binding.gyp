{
  "targets": [{
    "target_name": "unlisten",
    "sources": [
      "./src/unlisten.cc"
    ],
    "include_dirs" : [
	"<!(node -e \"require('nan')\")"
    ]
  }],
}
