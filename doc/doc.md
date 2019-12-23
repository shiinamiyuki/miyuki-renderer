# Docs

## Design

miyuki-renderer is designed to be a modern physically based renderer. 

The renderer is designed with the following goals in mind:

- User friendliness
- Physically based/Optional NPR
- Advanced light transport algorithms
- Extensibility

## Scene File Format

We use a json based scene description file.



## Coding Style

The renderer is written in a not quite conventional object-oriented approach. We encourage the use of interfaces rather than abstract inheritance and we discourage the use of implementation inheritance. (That's why MYK_DECL_CLASS force your implementation to be final).

