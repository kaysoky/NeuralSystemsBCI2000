function [Montage]=load_bcimnt(MntFile)

%---Defining an empty struct---
Montage=[];

%---Randomly arranged Montage file Reader---%
k=1;
j=1;

fid=fopen(MntFile,'rt');

if fid ~= -1
    while ~feof(fid)
        singleline=fgetl(fid);
        if ~strcmp(unique(singleline),' ') && ~isempty(singleline)
            if ~isempty(strfind(singleline,'[labels]'))
                while ~feof(fid)
                    posinfile1=ftell(fid);
                    singleline=fgetl(fid);
                    if ~strcmp(unique(singleline),' ') && ~isempty(singleline)
                        if length(findstr('[name]',singleline))|length(findstr('[grid]',singleline))|length(findstr('[pos]',singleline))|length(findstr('[valid]',singleline))
                            fseek(fid,posinfile1,'bof');
                            k=1;
                            j=1;
                            break;
                        end

                        s = textscan(singleline, '%d %s');
                        Montage.ElectrodeLabels(:,s{1})=s{2};
                    end

                end% while ~feof(fid)
            %case of small and large laplacian
            elseif strcmpi(singleline,'[grid]')
                while ~feof(fid)
                    posinfile2=ftell(fid);
                    singleline=fgetl(fid);
                    if ~strcmp(unique(singleline),' ') && ~isempty(singleline)

                        if length(findstr('[name]',singleline))|length(findstr('[labels]',singleline))|length(findstr('[pos]',singleline))|length(findstr('[valid]',singleline))
                            fseek(fid,posinfile2,'bof');
                            k=1;
                            j=1;
                            break;
                        else
                            s = textscan(singleline, '%d');
                            for k=1:length(s) %size(numChinRow,2)-1
                                Montage.SpatialGrid(j,:)=s{1};
                            end
                            j=j+1;
                        end
                    end
                end%while ~feof(fid)

            elseif strcmpi(singleline,'[name]')
                while ~feof(fid)
                    posinfile2=ftell(fid);
                    singleline=fgetl(fid);
                    if ~strcmp(unique(singleline),' ') && ~isempty(singleline)

                        if length(strfind('[grid]',singleline))|length(strfind('[labels]',singleline))|length(strfind('[pos]',singleline))|length(strfind('[valid]',singleline))
                            fseek(fid,posinfile2,'bof');
                            k=1;
                            j=1;
                            break;
                        else
                            if ~isempty(singleline)
                                Montage.SyntheticName = singleline;
                            end
                            j=j+1;
                        end
                    end
                end%while ~feof(fid)

            elseif strcmpi(singleline,'[pos]')
                while ~feof(fid)
                    posinfile2=ftell(fid);
                    singleline=fgetl(fid);
                    if ~strcmp(unique(singleline),' ') && ~isempty(singleline)

                        if length(strfind('[grid]',singleline))|length(strfind('[labels]',singleline))|length(strfind('[name]',singleline))|length(strfind('[valid]',singleline))
                            fseek(fid,posinfile2,'bof');
                            k=1;
                            j=1;
                            break;
                        else
                            s = textscan(singleline, '%d %f %f %f');
                            Montage.SpatialPositions(s{1},:) = [s{2} s{3} s{4}];
                            j=j+1;
                        end
                    end
                end%while ~feof(fid)

            elseif strcmpi(singleline,'[valid]')
                while ~feof(fid)
                    posinfile2=ftell(fid);
                    singleline=fgetl(fid);
                    if ~strcmp(unique(singleline),' ') && ~isempty(singleline)

                        if length(strfind('[grid]',singleline))|length(strfind('[labels]',singleline))|length(strfind('[name]',singleline))|length(strfind('[pos]',singleline))
                            fseek(fid,posinfile2,'bof');
                            k=1;
                            j=1;
                            break;
                        else
                            s = textscan(singleline, '%d %d');
                            Montage.ValidChannels(s{1}) = s{2};
                            j=j+1;
                        end
                    end
                end%while ~feof(fid)
            end
        end
    end%while ~feof(fid)

    fclose(fid);
    Montage.ValidChannels = logical(Montage.ValidChannels);
    if ~isfield(Montage,'SyntheticName')
        Montage.SyntheticName = MntFile;
    end
end
%===========Fine ciclo lettura file di testo del montaggio==================