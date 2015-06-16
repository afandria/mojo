
import 'package:vector_math/vector_math.dart';

import '../rendering/block.dart';
import '../rendering/box.dart';
import '../rendering/flex.dart';
import '../rendering/object.dart';
import '../rendering/paragraph.dart';
import '../rendering/stack.dart';
import 'ui_node.dart';

export '../rendering/box.dart' show BoxConstraints, BoxDecoration, Border, BorderSide, EdgeDims;
export '../rendering/flex.dart' show FlexDirection, FlexJustifyContent, FlexAlignItems;
export '../rendering/object.dart' show Point, Size, Rect, Color, Paint, Path;
export 'ui_node.dart' show UINode, Component, App, EventListenerNode, ParentDataNode;


// PAINTING NODES

class Opacity extends OneChildRenderObjectWrapper {
  Opacity({ this.opacity, UINode child, Object key })
    : super(child: child, key: key);

  RenderOpacity get root { RenderOpacity result = super.root; return result; }
  final double opacity;

  RenderOpacity createNode() => new RenderOpacity(opacity: opacity);

  void syncRenderObject(Opacity old) {
    super.syncRenderObject(old);
    root.opacity = opacity;
  }
}

class DecoratedBox extends OneChildRenderObjectWrapper {

  DecoratedBox({ this.decoration, UINode child, Object key })
    : super(child: child, key: key);

  RenderDecoratedBox get root { RenderDecoratedBox result = super.root; return result; }
  final BoxDecoration decoration;

  RenderDecoratedBox createNode() => new RenderDecoratedBox(decoration: decoration);

  void syncRenderObject(DecoratedBox old) {
    super.syncRenderObject(old);
    root.decoration = decoration;
  }

}

class CustomPaint extends OneChildRenderObjectWrapper {

  CustomPaint({ this.callback, this.token, UINode child, Object key })
    : super(child: child, key: key);

  RenderCustomPaint get root { RenderCustomPaint result = super.root; return result; }
  final CustomPaintCallback callback;
  final dynamic token;  // set this to be repainted automatically when the token changes

  RenderCustomPaint createNode() => new RenderCustomPaint(callback: callback);

  void syncRenderObject(CustomPaint old) {
    super.syncRenderObject(old);
    if (old != null && old.token != token)
      root.markNeedsPaint();
    root.callback = callback;
  }

  void remove() {
    root.callback = null;
    super.remove();
  }

}

class ClipRect extends OneChildRenderObjectWrapper {

  ClipRect({ UINode child, Object key })
    : super(child: child, key: key);

  RenderClipRect get root { RenderClipRect result = super.root; return result; }
  RenderClipRect createNode() => new RenderClipRect();
}

class ClipOval extends OneChildRenderObjectWrapper {

  ClipOval({ UINode child, Object key })
    : super(child: child, key: key);

  RenderClipOval get root { RenderClipOval result = super.root; return result; }
  RenderClipOval createNode() => new RenderClipOval();
}


// POSITIONING AND SIZING NODES

class Transform extends OneChildRenderObjectWrapper {

  Transform({ this.transform, UINode child, Object key })
    : super(child: child, key: key);

  RenderTransform get root { RenderTransform result = super.root; return result; }
  final Matrix4 transform;

  RenderTransform createNode() => new RenderTransform(transform: transform);

  void syncRenderObject(Transform old) {
    super.syncRenderObject(old);
    root.transform = transform;
  }

}

class Padding extends OneChildRenderObjectWrapper {

  Padding({ this.padding, UINode child, Object key })
    : super(child: child, key: key);

  RenderPadding get root { RenderPadding result = super.root; return result; }
  final EdgeDims padding;

  RenderPadding createNode() => new RenderPadding(padding: padding);

  void syncRenderObject(Padding old) {
    super.syncRenderObject(old);
    root.padding = padding;
  }

}

class Center extends OneChildRenderObjectWrapper {

  Center({ UINode child, Object key })
    : super(child: child, key: key);

  RenderPositionedBox get root { RenderPositionedBox result = super.root; return result; }

  RenderPositionedBox createNode() => new RenderPositionedBox();

}

class SizedBox extends OneChildRenderObjectWrapper {

  SizedBox({
    this.width,
    this.height,
    UINode child,
    Object key
  }) : super(child: child, key: key);

  RenderConstrainedBox get root { RenderConstrainedBox result = super.root; return result; }

  final double width;
  final double height;

  RenderConstrainedBox createNode() => new RenderConstrainedBox(additionalConstraints: _additionalConstraints());

  BoxConstraints _additionalConstraints() {
    var result = const BoxConstraints();
    if (width != null)
      result = result.applyWidth(width);
    if (height != null)
      result = result.applyHeight(height);
    return result;
  }

  void syncRenderObject(SizedBox old) {
    super.syncRenderObject(old);
    root.additionalConstraints = _additionalConstraints();
  }

}

class ConstrainedBox extends OneChildRenderObjectWrapper {

  ConstrainedBox({ this.constraints, UINode child, Object key })
    : super(child: child, key: key);

  RenderConstrainedBox get root { RenderConstrainedBox result = super.root; return result; }
  final BoxConstraints constraints;

  RenderConstrainedBox createNode() => new RenderConstrainedBox(additionalConstraints: constraints);

  void syncRenderObject(ConstrainedBox old) {
    super.syncRenderObject(old);
    root.additionalConstraints = constraints;
  }

}

class ShrinkWrapWidth extends OneChildRenderObjectWrapper {

  ShrinkWrapWidth({ UINode child, Object key }) : super(child: child, key: key);

  RenderShrinkWrapWidth get root { RenderShrinkWrapWidth result = super.root; return result; }

  RenderShrinkWrapWidth createNode() => new RenderShrinkWrapWidth();

}

class SizeObserver extends OneChildRenderObjectWrapper {

  SizeObserver({ this.callback, UINode child, Object key })
    : super(child: child, key: key);

  RenderSizeObserver get root { RenderSizeObserver result = super.root; return result; }
  final SizeChangedCallback callback;

  RenderSizeObserver createNode() => new RenderSizeObserver(callback: callback);

  void syncRenderObject(SizeObserver old) {
    super.syncRenderObject(old);
    root.callback = callback;
  }

  void remove() {
    root.callback = null;
    super.remove();
  }

}


// CONVENIENCE CLASS TO COMBINE COMMON PAINTING, POSITIONING, AND SIZING NODES

class Container extends Component {

  Container({
    Object key,
    this.child,
    this.constraints,
    this.decoration,
    this.width,
    this.height,
    this.margin,
    this.padding,
    this.transform
  }) : super(key: key);

  final UINode child;
  final BoxConstraints constraints;
  final BoxDecoration decoration;
  final EdgeDims margin;
  final EdgeDims padding;
  final Matrix4 transform;
  final double width;
  final double height;

  UINode build() {
    UINode current = child;

    if (child == null && width == null && height == null)
      current = new SizedBox(
        width: double.INFINITY,
        height: double.INFINITY
      );

    if (padding != null)
      current = new Padding(padding: padding, child: current);

    if (decoration != null)
      current = new DecoratedBox(decoration: decoration, child: current);

    if (width != null || height != null)
      current = new SizedBox(
        width: width,
        height: height,
        child: current
      );

    if (constraints != null)
      current = new ConstrainedBox(constraints: constraints, child: current);

    if (margin != null)
      current = new Padding(padding: margin, child: current);

    if (transform != null)
      current = new Transform(transform: transform, child: current);

    return current;
  }

}


// LAYOUT NODES

class Block extends MultiChildRenderObjectWrapper {

  Block(List<UINode> children, { Object key })
    : super(key: key, children: children);

  RenderBlock get root { RenderBlock result = super.root; return result; }
  RenderBlock createNode() => new RenderBlock();

}

class Stack extends MultiChildRenderObjectWrapper {

  Stack(List<UINode> children, { Object key })
    : super(key: key, children: children);

  RenderStack get root { RenderStack result = super.root; return result; }
  RenderStack createNode() => new RenderStack();

}

class Positioned extends ParentDataNode {
  Positioned({
    UINode child,
    double top,
    double right,
    double bottom,
    double left
  }) : super(content, new StackParentData()..top = top
                                           ..right = right
                                           ..bottom = bottom
                                           ..left = left);
}

class Flex extends MultiChildRenderObjectWrapper {

  Flex(List<UINode> children, {
    Object key,
    this.direction: FlexDirection.horizontal,
    this.justifyContent: FlexJustifyContent.flexStart,
    this.alignItems: FlexAlignItems.center
  }) : super(key: key, children: children);

  RenderFlex get root { RenderFlex result = super.root; return result; }
  RenderFlex createNode() => new RenderFlex(direction: this.direction);

  final FlexDirection direction;
  final FlexJustifyContent justifyContent;
  final FlexAlignItems alignItems;

  void syncRenderObject(UINode old) {
    super.syncRenderObject(old);
    root.direction = direction;
    root.justifyContent = justifyContent;
    root.alignItems = alignItems;
  }

}

class Flexible extends ParentDataNode {
  Flexible({ UINode child, int flex: 1, Object key })
    : super(child, new FlexBoxParentData()..flex = flex, key: key);
}

class Paragraph extends RenderObjectWrapper {

  Paragraph({ Object key, this.text, this.style }) : super(key: key);

  RenderParagraph get root { RenderParagraph result = super.root; return result; }
  RenderParagraph createNode() => new RenderParagraph(text: text, style: style);

  final String text;
  final TextStyle style;

  void syncRenderObject(UINode old) {
    super.syncRenderObject(old);
    root.text = text;
    root.style = style;
  }

  void insert(RenderObjectWrapper child, dynamic slot) {
    assert(false);
    // Paragraph does not support having children currently
  }

}

class Text extends Component {
  Text(this.data, { TextStyle this.style }) : super(key: '*text*');
  final String data;
  final TextStyle style;
  bool get interchangeable => true;
  UINode build() => new Paragraph(text: data, style: style);
}

class Image extends RenderObjectWrapper {

  Image({
    Object key,
    this.src,
    this.size
  }) : super(key: key);

  RenderImage get root { RenderImage result = super.root; return result; }
  RenderImage createNode() => new RenderImage(this.src, this.size);

  final String src;
  final Size size;

  void syncRenderObject(UINode old) {
    super.syncRenderObject(old);
    root.src = src;
    root.requestedSize = size;
  }

  void insert(RenderObjectWrapper child, dynamic slot) {
    assert(false);
    // Image does not support having children currently
  }

}
