from ultralytics.utils.benchmarks import benchmark

benchmark(model="yolo26m.pt", data="coco8.yaml", imgsz=640, format="onnx", half=True)
benchmark(model="yolo26m.pt", data="coco8.yaml", imgsz=640, format="ncnn", half=True)
benchmark(model="yolo26m.pt", data="coco8.yaml", imgsz=640, format="-", half=True)